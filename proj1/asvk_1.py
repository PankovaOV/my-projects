import math
import heapq
import json
from collections import defaultdict


def distance(a, b):
    return math.sqrt((a["x"] - b["x"]) ** 2 + (a["y"] - b["y"]) ** 2)


def read_input(filename):
    with open(filename, 'r') as file:
        data = json.load(file)
    towers = {}  # словарь для хранения вышек
    connections = []  # список соединений между вышками
    # проверки полей вершин
    if 'towers' not in data:
        raise ValueError("Файл должен содержать ключ 'towers'")
    if 'connections' not in data:
        raise ValueError("Файл должен содержать ключ 'connections'")

    for tower in data['towers']:
        if 'id' not in tower or 'coordinates' not in tower:
            raise ValueError("Каждая вышка должна иметь 'id' и 'coordinates'")
        tower_id = tower['id']
        if tower_id in towers:
            raise ValueError(f"Дубликат id вышки: {tower_id}")

        # преобразование координат в числа и сохранение в словарь вышек
        try:
            x = float(tower['coordinates']['x'])
            y = float(tower['coordinates']['y'])
        except (ValueError, KeyError):
            raise ValueError("Координаты должны быть числами")

        towers[tower_id] = {"x": x, "y": y}
    # обработка соединений
    for connection in data['connections']:
        if 'from' not in connection or 'to' not in connection:
            continue
        from_tower = connection['from']
        to_tower = connection['to']
        if from_tower not in towers or to_tower not in towers:
            print(f"Соединение между несуществующими вышками {from_tower} и {to_tower}")
            continue
        if from_tower == to_tower:
            print(f"Соединение вышки {from_tower} с самой собой")
            continue

        connections.append((from_tower, to_tower))
    return towers, connections


# алгоритм Прима
def min_span_graph(towers, connections):
    if not towers:
        return {"minimum_spanning_tree": [], "total_cost": 0.0}

    # граф - словарь смежности
    graph = defaultdict(list)
    for from_tower, to_tower in connections:
        # вычисление расстояния между вышками
        dist = distance(towers[from_tower], towers[to_tower])
        graph[from_tower].append((to_tower, dist))
        graph[to_tower].append((from_tower, dist))
    # проверка связности
    if len(graph) < len(towers):
        print("Не все вышки связаны между собой")

    # алгоритм Прима с использованием кучи
    start_node = next(iter(towers)) # начальная вышка
    visited = {start_node} # посещенные вышки
    mst_edges = [] # ребра минимального остовного графа
    total_cost = 0.0 # их суммарная длина
    # инициализация кучи с ребрами начальной вершины
    edges = [(dist, start_node, to_tower) for to_tower, dist in graph[start_node]]
    heapq.heapify(edges)

    while edges and len(visited) < len(towers):
        # извлечение ребра минимального расстояния
        dist, from_tower, to_tower = heapq.heappop(edges)
        if to_tower not in visited:
            visited.add(to_tower)
            mst_edges.append({"from": from_tower, "to": to_tower})
            total_cost += dist
            # добавление всех ребер новой вершины в кучу
            for neighbor, neighbor_dist in graph[to_tower]:
                if neighbor not in visited:
                    heapq.heappush(edges, (neighbor_dist, to_tower, neighbor))
    return {
        "minimum_spanning_tree": mst_edges,
        "total_cost": total_cost
    }


if __name__ == "__main__":
    try:
        input_filename = input("Введите имя входного JSON-файла:\n").strip()
        towers, connections = read_input(input_filename)
        result = min_span_graph(towers, connections)
        print("Результат:")
        print(json.dumps(result, indent=5))
    except FileNotFoundError:
        print("Ошибка: файл не найден")
    except json.JSONDecodeError:
        print("Ошибка: файл должен быть в формате JSON")
