import requests
import json
import time

SERVER_URL = "http://server:5000"


def send_request(file_path):
    with open(file_path) as f:
        data = json.load(f)

    response = requests.post(
        f"{SERVER_URL}/process",
        json=data,
        headers={'Content-Type': 'application/json'}
    )

    return data, response.json()


def run_system_tests():
    while True:
        for i in range(1, 11):
            test_file = f"test{i}.json"
            input_file, result = send_request(test_file)
            print("\n" + "=" * 50)
            print(f"Тестирование файла: {test_file}")
            print("=" * 50)
            print("\nВходные данные:\n")
            print(json.dumps(input_file, indent=5, ensure_ascii=False))
            if result:
                print("\nВыходные данные:\n")
                print(json.dumps(result, indent=5, ensure_ascii=False))
            time.sleep(5)


if __name__ == '__main__':
    run_system_tests()