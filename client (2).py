# -*- coding: utf-8 -*-
import requests, json, time

URL = 'http://192.168.43.171:5000'

#ввод количества игроков
resp = json.loads(requests.post(URL + '/players_count', json={"players_count": 'g'}).text[:-1])
if 'Количество игроков уже было выбрано другим' not in resp:
    while True:
        print('\nВведите количество игроков:')
        data = {"players_count": input()}
        resp = json.loads(requests.post(URL + '/players_count', json=data).text[:-1])
        print(resp)
        if 'игроков установлено' in resp or 'игроков уже было выбрано' in resp:
            break
            
#регистрация игрока
while True:
    print('\n\nВведите своё имя:')
    data = {"name": input()}
    resp = json.loads(requests.post(URL + '/auth', json=data).text[:-1]) 
    print(resp)
    if 'максимальное' in resp:
        exit()
    elif 'Готово' in resp:
        break

#ожидание остальных игроков
requests.get(URL + '/check_start').text[:-1]

#ввод режима игры
print('Игра начинается!')
resp = requests.post(URL + '/game_mode', json={"mode": 'g'}).text[0]
while True:
    print('\nЕсли хотите играть до банкротства, введите 0, если по количеству месяцев - введите количество месяцев:')
    data = {"mode": input()}
    resp = json.loads(requests.post(URL + '/game_mode', json=data).text[:-1])
    print(resp)
    if 'Режим игры уже был выбран другим' in resp or 'Режим игры установлен' in resp:
        time.sleep(2)
        break

#игра
while True:
    resp = json.loads(requests.post(URL + '/game').text[:-1])
    print(resp)
    if 'Игра завершена' in resp:
        exit()
    elif "СОСТОЯНИЕ" in resp:
        while True:
            flag = False
            data = {"hod": input()}
            resp = json.loads(requests.post(URL + '/hod', json=data).text[:-1]) 
            print(resp)
            if 'желаемое количество единиц сырья' in resp:
                while True:
                    data = {"count_metall": input()}
                    resp = json.loads(requests.post(URL + '/z_metall', json=data).text[:-1])
                    print(resp)
                    if '\n\n\n\n\n\n\n\n\n\n' in resp:
                        flag = True
                        break
                    elif 'Введите цену, которую вы готовы потратить на единицу сырья' in resp:
                        break
                if flag == True:
                    break
                while True:
                    data = {"price_metall": input()}
                    resp = json.loads(requests.post(URL + '/z_metall_price', json=data).text[:-1]) 
                    print(resp)
                    if '\n\n\n\n\n\n\n\n\n\n' in resp:
                        break
                break
            elif 'нет истребителей' in resp or 'максимальное количество цехов' in resp or 'денег на покупку цеха' in resp or 'уже находится в процессе строительства' in resp:
                time.sleep(10)
                break
            elif 'количество истребителей, которые вы хотите продать' in resp:
                while True:
                    data = {"count_sell_planes": input()}
                    resp = json.loads(requests.post(URL + '/z_sell_istr', json=data).text[:-1])
                    print(resp)
                    if '\n\n\n\n\n\n\n\n\n\n' in resp:
                        flag = True
                        break
                    elif 'Введите цену, которую вы хотите получить за 1 истребитель' in resp:
                        break
                if flag == True:
                    break    
                while True:
                    data = {"price_sale_planes": input()}
                    resp = json.loads(requests.post(URL + '/z_sell_istr_price', json=data).text[:-1]) 
                    print(resp)
                    if '\n\n\n\n\n\n\n\n\n\n' in resp:
                        break
                break
            elif '1, 2, 3, 4' in resp:
                continue
            elif 'Игра завершена' in resp:
                exit()
            elif 'вы хотите построить цех' in resp:
                while True:
                    data = {"workshop_yn": input()}
                    resp = json.loads(requests.post(URL + '/z_workshop', json=data).text[:-1])
                    print(resp)
                    if '\n\n\n\n\n\n\n\n\n\n' in resp:
                        break             
                break
            elif 'колько истребителей вы хотите построить' in resp:
                while True:
                    data = {"planes_build": input()}
                    resp = json.loads(requests.post(URL + '/z_build_istr', json=data).text[:-1])
                    print(resp)
                    if '\n\n\n\n\n\n\n\n\n\n' in resp:
                        break             
                break         
            elif 'пока остальные игроки закончат' in resp:
                json.loads(requests.post(URL + '/check_end').text[:-1])
                break
    elif 'вы банкрот' in resp:
        resp = json.loads(requests.get(URL + '/wait').text[:-1])
        print(resp)        
        exit()
    elif 'РАСПРОДАЖА' in resp:
        print(resp)
        time.sleep(15)