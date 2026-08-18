from flask import Flask, request, jsonify
import math
import heapq
from collections import defaultdict


def distance(a, b):
    return math.sqrt((a["x"] - b["x"])**2 + (a["y"] - b["y"])**2)


app = Flask(__name__)


@app.route('/process', methods=['POST', 'GET'])
def process_request():
    # GET оказался необходим в работе с Play with Docker
    if request.method == 'GET':
        return 'test PWD'
    data = request.json
    if 'towers' not in data or 'connections' not in data:
        return jsonify({
            "error": "Invalid data structure",
            "message": "Файл должен содержать ключи 'towers' и 'connections'"
        }), 400

    towers = {}
    invalid_connections = []
    for tower in data['towers']:
        if 'id' not in tower or 'coordinates' not in tower:
            return jsonify({
                "error": "Invalid tower data",
                "message": "Каждая вышка должна иметь 'id' и 'coordinates'"
            }), 400

        try:
            x = float(tower['coordinates']['x'])
            y = float(tower['coordinates']['y'])
        except (ValueError, KeyError):
            return jsonify({
                "error": "Invalid coordinates",
                "message": "Координаты должны быть числами в формате {'x': число, 'y': число}"
            }), 400

        if tower['id'] in towers:
            return jsonify({
                "error": "Duplicate tower id",
                "message": f"Дубликат id вышки: {tower['id']}"
            }), 400

        towers[tower['id']] = {"x": x, "y": y}

    graph = defaultdict(list)

    for connection in data['connections']:
        if 'from' not in connection or 'to' not in connection:
            continue
        from_tower = connection['from']
        to_tower = connection['to']
        if from_tower not in towers or to_tower not in towers:
            invalid_connections.append(f"Соединение между несуществующими вышками {from_tower} и {to_tower}")
            continue

        if from_tower == to_tower:
            invalid_connections.append(f"Соединение вышки {from_tower} с самой собой")
            continue

        dist = distance(towers[from_tower], towers[to_tower])
        graph[from_tower].append((to_tower, dist))
        graph[to_tower].append((from_tower, dist))

    if not graph:
        return jsonify({
            "minimum_spanning_tree": [],
            "total_cost": 0.0,
            "warnings": invalid_connections
        })

    start_node = next(iter(towers))
    visited = {start_node}
    mst_edges = []
    total_cost = 0.0
    edges = [(dist, start_node, to_tower) for to_tower, dist in graph[start_node]]
    heapq.heapify(edges)

    while edges and len(visited) < len(towers):
        dist, from_tower, to_tower = heapq.heappop(edges)
        if to_tower not in visited:
            visited.add(to_tower)
            mst_edges.append({"from": from_tower, "to": to_tower})
            total_cost += dist
            for neighbor, neighbor_dist in graph[to_tower]:
                if neighbor not in visited:
                    heapq.heappush(edges, (neighbor_dist, to_tower, neighbor))
    if len(visited) < len(towers):
        invalid_connections.append("Не все вышки связаны между собой")

    return jsonify({
        "minimum_spanning_tree": mst_edges,
        "total_cost": total_cost,
        "warnings": invalid_connections if invalid_connections else None
    })


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
