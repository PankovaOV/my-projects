# -*- coding: utf-8 -*-

from flask import Flask, request, jsonify
import random, time, socket

app = Flask(__name__)

#распродажа сырья, авиасалон, переход рынка на новый уровень, очистка списка прежложений
metall_offer = [1, 1.5, 2, 2.5, 3]
min_price = [800, 650, 500, 400, 300]
spros = [3, 2.5, 2, 1.5, 1]
max_price = [6500, 6000, 5500, 5000, 4500]
new_lvl = [[1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4, 5], [1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 5], [1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5], [1, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5], [1, 2, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5]]

#0 - ход игрока, 1 - покупка сырья и авиасалон
game_stade = 0
market_stade = 3
winner_game = ''
months = 0
applications_metall_price = []
applications_metall_count = []
applications_planes_price = []
applications_planes_count = []
ready_players = []

#функция, находящая ip сервера
def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP

#функция проверки числа, целое ли оно и входит ли оно в промежуток
def isint(s, *args):
    try:
        int(s)
        #если нет аргументов на вход, то проверяем на положительность/0
        if len(args) == 0 and int(s) >= 0:
            return True
        #иначе проверяем есть ли вводимое число в списке возможных аргументов
        elif int(s) in args:
            return True
        else:
            return False
    except ValueError:
        return False


class Player():
    def __init__(self, name):
        self.name = name
        self.money = 10000
        self.metall = 4
        self.plane = 2
        self.workshop = 2
        self.building = 0
        
        self.nalog = []
        
        self.z_build_istr = 0
        self.z_workshop = 0
        self.z_metall = [0, 0]
        self.z_sell_istr = [0, 0]        

    def build_new_workshop(self):
        self.building = months + 4
  
#ввод количества игроков
player_count = -1
@app.route('/players_count', methods=['POST'])
def set_player_count():
    global player_count
    if player_count == -1:
        user = request.get_json()['players_count']
        if isint(user, *list(range(2, 101))):
            player_count = int(user)
            return jsonify('\nКоличество игроков установлено!')
        else:
            return jsonify('\nВведите только число от 2 до 100:')
    else:
        return jsonify('\nКоличество игроков уже было выбрано другим игроком')
    
players = []
ips = []
@app.route('/auth', methods=['POST'])
def add_player():
    global players, player_count
    if len(players) >= player_count:
        return jsonify('В игре уже максимальное количество игроков!')
    else:
        user = request.get_json()['name']
        if len(user) == 0:
            return jsonify('Имя игрока должно содержать хотя бы 1 символ!')
        else:
            players.append(Player(user))
            ips.append(request.remote_addr)
            return jsonify('\nГотово, вы в игре!\nОжидание подключения остальных игроков...')

@app.route('/check_start', methods=['GET'])
def check_start():
    global players, player_count
    while len(players) < player_count:
        continue
    return jsonify(0)

l = 0
@app.route('/check_end', methods=['POST'])  
def check_end():
    global ready_players, game_stade, l, players
    
    while len(ready_players) < len(players) and l == 0:      
        continue
    time.sleep(0.5)
    game_stade = 1
    l = 1
    for_printing = ''
    return jsonify(0)
mode = -1

@app.route('/game_mode', methods=['POST'])
def set_game_mode():
    global mode
    if mode == -1:
        user = request.get_json()['mode']
        if isint(user):
            mode = int(user)
            return jsonify('\nРежим игры установлен')
        else:
            return jsonify('\nНапишите только число')
    else:
        return jsonify('\nРежим игры уже был выбран другим игроком')

opana = 0
def check_winner():
    global winner_game, players, mode, months, opana
    
    if winner_game != '':
        return True
    elif mode == 0 and len(players) == 1:
        winner_game = '\nИгра завершена. Победил ' + players[0].name + '. Поздравляем!'
        return True
    elif mode == 0 and len(players) == 0:
        winner_game = '\nИгра завершена. Никто не дожил до конца. Несколько игроков обанкротились одновременно.'
        return True
    elif mode != 0 and months == mode:
        if opana == 0:
            opana = 1
            res = []
            for elem in players:
                ost = elem.workshop * 5000
                if elem.building != 0:
                    ost += 5000
                    ost -= 2500
                ost += elem.metall * (min_price[market_stade - 1])
                ost += elem.plane * (max_price[market_stade - 1])
                ost += elem.money
                res.append(ost)    
            winner_game = '\nИгра завершена. Победил ' + players[res.index(max(res))].name + '. Поздравляем!'
        return True
    else:
        return False
        
k = 0
for_printing = ''
@app.route('/game', methods=['POST'])
def game():
    global winner, applications_metall_price, applications_metall_count, applications_planes_price, applications_planes_count, game_stade, market_stade, metall_offer, new_lvl, spros, ready_players, winner_game, months, for_printing, k, l
    
    #проверяем, не закончилась ли игра и объявляем победителя
    if game_stade == 0:
        #списывание налогов
        k = 0
        human = players[ips.index(request.remote_addr)]
        if months != 0 and months not in human.nalog:
            human.money -= (human.metall * 300 + human.plane * 500 + human.workshop * 1000)
            if human.building != 0 and months == human.building:
                print('Сейчас месяц', months, 'надо - ', human.building)
                human.building = 0
                human.workshop += 1
                human.money -= 2500 #2 половина за строительство  
            human.nalog.append(months)
        if human.money < 0:
            del players[ips.index(request.remote_addr)]
            del ips[ips.index(request.remote_addr)]
            return jsonify('\nУвы, вы банкрот! Состояние счёта: ' + str(human.money) + '\nВам удалось продержаться месяцев: ' + str(months) + '  ')
        else:
            time.sleep(0.5)
            if check_winner():
                return jsonify(winner_game)
            else:
                return jsonify(print_info(human))
    else:
        if k == 0:
            k = 1
            for_printing = '\n' * 100 + 'РАСПРОДАЖА СЫРЬЯ\n'
            #продажа сырья
            n = int(len(players) * metall_offer[market_stade - 1])
            ready_players1 = ready_players.copy()
            while n > 0:
                best_price = max(applications_metall_price)
                if best_price == 0:
                    break
                if applications_metall_price.count(best_price) > 1:
                    candidates = []
                    for i in range(len(applications_metall_price)):
                        if applications_metall_price[i] == best_price:
                            candidates.append(i)         
                    index_win = candidates[random.randrange(len(candidates))]
                else:
                    index_win = applications_metall_price.index(best_price)  
                winner = ready_players1[index_win]
                for i in range(applications_metall_count[index_win]):
                    if n > 0 and winner.money >= best_price:
                        #добавляем победителю материалы
                        winner.metall += 1
                        #забираем у победителя деньги
                        winner.money -= best_price
                        for_printing += '\nИгрок ' + winner.name + ' получает 1 единицу сырья'
                        n -= 1             
                    else:
                        break
                del ready_players1[index_win]
                del applications_metall_count[index_win]
                del applications_metall_price [index_win]  
            for_printing += '\n\nАукцион завершен!\n\nАВИАСАЛОН\n'
            #авиасалон
            n = int(len(players) * spros[market_stade - 1])
            while 0 in applications_planes_count:
                zero = applications_planes_count.index(0)
                del ready_players[zero]
                del applications_planes_count[zero]
                del applications_planes_price[zero]  
            while n > 0 and len(applications_planes_count) > 0:
                best_price = min(applications_planes_price) 
                if applications_planes_price.count(best_price) > 1:
                    candidates = []
                    for i in range(len(applications_planes_price)):
                        if applications_planes_price[i] == best_price:
                            candidates.append(i)   
                    index_win = candidates[random.randrange(len(candidates))]
                else:
                    index_win = applications_planes_price.index(best_price)
                winner = ready_players[index_win]
                for i in range(applications_planes_count[index_win]):
                    if n > 0 and winner.plane >= 1:
                        #добавляем победителю материалы
                        winner.plane -= 1
                        #забираем у победителя деньги
                        winner.money += best_price
                        for_printing += '\nИгрок ' + winner.name + ' продает 1 истребитель'
                        n -= 1 
                    else:
                        break
                del ready_players[index_win]
                del applications_planes_count[index_win]
                del applications_planes_price[index_win]  
            for_printing += '\n\nАвиасалон завершен!'
            for_printing += '\n\nСледующий месяц начнется через 15 секунд!'
            
            applications_metall_price = []
            applications_metall_count = []
            applications_planes_price = []
            applications_planes_count = []
            ready_players = []        
            market_stade = new_lvl[market_stade - 1][random.randrange(12)]
            months += 1
        time.sleep(0.5)
        game_stade = 0
        l = 0
        k = 0
        return jsonify(for_printing)


@app.route('/z_metall', methods=['POST'])
def give_metall():
    user = request.get_json()['count_metall']
    if not isint(user, *list(range(metall_offer[market_stade - 1] * len(players) + 1))):
        return jsonify('\nВведите число - количество сырья от 1 до ' + str(metall_offer[market_stade - 1] * len(players)) + ' (количество предложений) или 0 чтобы отменить покупку:')
    elif user == '0':
        players[ips.index(request.remote_addr)].z_metall[0] = 0
        players[ips.index(request.remote_addr)].z_metall[1] = 0
        return jsonify('\n' * 100)
    else:
        players[ips.index(request.remote_addr)].z_metall[0] = int(user)
        return jsonify('\nВведите цену, которую вы готовы потратить на единицу сырья или 0, чтобы отменить покупку:')
    
@app.route('/z_metall_price', methods=['POST'])
def give_metall_price():
    user = request.get_json()['price_metall']
    if not isint(user):
        return jsonify('\nВведите только число - цену за единицу сырья или 0:')
    elif user == '0':
        players[ips.index(request.remote_addr)].z_metall[0] = 0
        players[ips.index(request.remote_addr)].z_metall[1] = 0        
        return jsonify('\n' * 100)
    elif int(user) * players[ips.index(request.remote_addr)].z_metall[0] > players[ips.index(request.remote_addr)].money:
        return jsonify('\nУ вас не хватит денег на покупку!')
    elif int(user) >= min_price[market_stade - 1]:
        players[ips.index(request.remote_addr)].z_metall[1] = int(user)
        return jsonify('\n' * 100)
    else:
        return jsonify('\nВведенная цена меньше минимальной цены на рынке (' + str(min_price[market_stade - 1]) + '). Введите цену еще раз:')
    
@app.route('/z_sell_istr', methods=['POST'])
def sell_istr():
    user = request.get_json()['count_sell_planes']
    human = players[ips.index(request.remote_addr)]
    if not isint(user, *list(range(human.plane + 1))):
        return jsonify('\nВведите число от 1 до ' + str(human.plane) + ' (количество самолетов у вас) или 0 чтобы отменить продажу\n')
    elif user == '0':
        human.z_sell_istr[0] = 0
        human.z_sell_istr[1] = 0
        return jsonify('\n' * 100)
    else:
        human.z_sell_istr[0] = int(user)
        return jsonify('\nВведите цену, которую вы хотите получить за 1 истребитель, или 0, чтобы отменить покупку:')
    
@app.route('/z_sell_istr_price', methods=['POST'])
def sell_istr_price():
    user = request.get_json()['price_sale_planes']
    if not isint(user):
        return jsonify('\nВведите только число - цену за единицу сырья или 0:')
    elif user == '0':
        players[ips.index(request.remote_addr)].z_sell_istr[0] = 0
        players[ips.index(request.remote_addr)].z_sell_istr[1] = 0
        return jsonify('\n' * 100)
    elif 0 < int(user) <= max_price[market_stade - 1]:
        players[ips.index(request.remote_addr)].z_sell_istr[1] = int(user)
        return jsonify('\n' * 100)
    else:
        return jsonify('\nЗаявленная цена слишком большая.\nВведите число от 0 до ' + str(max_price[market_stade - 1]) + ' (максимальная цена) или 0 чтобы отменить покупку:')

@app.route('/z_workshop', methods=['POST'])
def build_factory():
    user = request.get_json()['workshop_yn']
    if user == '0':
        players[ips.index(request.remote_addr)].z_workshop = 0
        return jsonify('\n' * 100)
    elif user == '1':
        players[ips.index(request.remote_addr)].z_workshop = 1
        return jsonify('\n' * 100)
    else:
        return jsonify('\nВведите только цифру 1 или 0:')

@app.route('/z_build_istr', methods=['POST'])
def build_planes():
    user = request.get_json()['planes_build']
    human = players[ips.index(request.remote_addr)]
    print(user)
    if not isint(user):
        return jsonify('\nВведите только число')
    if user == '0':
        human.z_build_istr = 0
        return jsonify('\n' * 100)
    elif int(user) > human.workshop:
        print(55)
        return jsonify('\nЖелаемое количество истребителей больше, чем количество цехов')
    elif int(user) * 2000 > human.money:
        return jsonify('\nУ вас не хватит денег на постройку истребителей!')  
    elif int(user) > human.metall:
        return jsonify('\nУ вас не хватит сырья на постройку истребителей!')    
    else:
        human.z_build_istr = int(user)
        return jsonify('\n' * 100)

@app.route('/wait', methods=['GET'])
def waiting_end():
    while winner_game == '':
        continue
    return jsonify(winner_game)

@app.route('/hod', methods=['POST'])
def hod():
    global applications_metall_price, applications_metall_count, applications_planes_count, applications_planes_price, players, spros, market_stade, ready_players, winner_game
    
    user = request.get_json()['hod']
    human = players[ips.index(request.remote_addr)]
    if mode == 0 and len(players) == 1:
        winner_game = '\nИгра завершена! Победил ' + players[0].name + '! Поздравляем!'
        return jsonify(winner_game)   
    elif mode == 0 and len(players) == 0:
        winner_game = '\nИгра завершена. Никто не дожил до конца. Несколько игроков обанкротились одновременно.'  
        return jsonify(winner_game)   
    elif mode != 0 and months == mode:
        res = []
        for elem in players:
            ost = elem.workshop * 5000
            if elem.building != 0:
                ost += 5000
                ost -= 2500
            ost += elem.metall * (min_price[market_stade - 1])
            ost += elem.plane * (max_price[market_stade - 1])
            ost += elem.money
            res.append(ost)    
        winner_game = '\nИгра завершена! Победил ' + players[res.index(max(res))].name + '! Поздравляем!' 
        return jsonify(winner_game) 
    elif user == '1':
        return jsonify('\n' * 100 + 'Предложений сырья: ' + str(int(len(players) * metall_offer[market_stade - 1])) + '\nМинимальная цена сырья: ' + str(min_price[market_stade - 1]) + ' т.р.\nСостояние вашего счета — ' + str(human.money) + ' т.р\n\nВведите желаемое количество единиц сырья или 0, чтобы отменить покупку:')
    elif user == '2':
        if human.plane == 0:
            return jsonify('\n' * 100 + 'У вас нет истребителей, вы не можете их продать!\n\n ')
        else:
            return jsonify('\n' * 100 + 'Количество готовых истребителей в наличии — ' + str(human.plane) + ' шт.\nВ этом месяце банк может купить максимум ' + str(int(len(players) * spros[market_stade - 1])) + ' истребителей\nМаксимальная цена истребителя: ' + str(max_price[market_stade - 1]) + ' т.р.\n\nВведите количество истребителей, которые вы хотите продать, или 0, чтобы отменить покупку:')
        
    elif user == '3':
        if human.workshop >= 6:
            return jsonify('\n' * 100 + 'У вас в пользовании максимальное количество цехов!\n\n')
        elif 5000 > human.money:
            return jsonify('\n' * 100 + 'У вас не хватит денег на покупку цеха!\n\n')
        elif human.building != 0:
            return jsonify('\n' * 100 + 'Другой цех уже находится в процессе строительства!\n\nДо окончания строительства осталось месяцев: ' + str(months - human.building))
        else:        
            return jsonify('\n' * 100 + 'Количество производственных цехов — ' + str(human.workshop) + '\nСостояние вашего счета — ' + str(human.money) + ' т.р\nЦена постройки цеха — 5000 т.р.\n\nЕсли вы хотите построить цех, введите 1, иначе — введите 0:')            
    elif user == '4':
        return jsonify('\n' * 100 + 'Количество производственных цехов — ' + str(human.workshop) + '\nКоличество сырья на складе — ' + str(human.metall) + ' ед.\n\nСостояние вашего счета — ' + str(human.money) + ' т.р\nЦена постройки истребителя — 2000 т.р.\n\nСколько истребителей вы хотите построить? Введите 0, чтобы отменить')        
    elif user == '0':
        if human.z_workshop == 1:
            human.build_new_workshop()
            human.money -= 2500 #1 половина за строительство 
        if human.z_build_istr != 0:
                human.plane += human.z_build_istr
                human.money -= 2000 * human.z_build_istr
                human.metall -= human.z_build_istr  
        
        applications_metall_price.append(human.z_metall[1])
        applications_metall_count.append(human.z_metall[0])
        applications_planes_price.append(human.z_sell_istr[1])
        applications_planes_count.append(human.z_sell_istr[0])
        ready_players.append(human)
        human.z_build_istr = 0
        human.z_workshop = 0
        human.z_metall = [0, 0]
        human.z_sell_istr = [0, 0]
        return jsonify('\n\nПодождите, пока остальные игроки закончат свой ход...')
    else:
        return jsonify('Введите только число 1, 2, 3, 4 или 0 чтобы завершить ход')
def print_info(human):
    if human.building != 0:
        return('\n' * 100 + '\nМесяцев прошло: ' + str(months) + '\n\nСОСТОЯНИЕ РЫНКА\n' + 'Предложений сырья: ' + str(int(len(players) * metall_offer[market_stade - 1])) + '\nМинимальная цена сырья: ' + str(min_price[market_stade - 1]) + ' т.р.\nСпрос на истребители: ' + str(int(len(players) * spros[market_stade - 1])) + '\nМаксимальная цена истребителя: ' + str(max_price[market_stade - 1]) + ' т.р.\n\nВАШЕ СОСТОЯНИЕ\nСостояние счета — ' + str(human.money) + ' т.р\nКоличество сырья на складе — ' + str(human.metall) + ' ед.\nКоличество готовых истребителей в наличии — ' + str(human.plane) + ' шт.\nКоличество производственных цехов — ' + str(human.workshop) + '\nДо постройки нового цеха осталось месяцев: ' + str(human.building - months) + '\n\nВЗАИМОДЕЙСТВИЕ\n1 - купить сырье у банка\nСЕЙЧАС в планах купить ' + str(human.z_metall[0]) + ' единиц по ' + str(human.z_metall[1]) + ' т.р. за шт.\n2 - продать истребитель банку\nСЕЙЧАС в планах продать ' + str(human.z_sell_istr[0]) + ' истребителей по ' + str(human.z_sell_istr[1]) + ' т.р. за истребитель\n3 - построить цех\n(цена постройки — 5000 т.р.)\nСЕЙЧАС вы хотите построить ' + str(human.z_workshop) + '\n\nПОСТРОЙКА ИСТРЕБИТЕЛЕЙ\nЦена постройки истребителя — 2000 т.р.\n4 - запустить сборку истребиелей\nСЕЙЧАС в планах построить ' + str(human.z_build_istr) + '\n\nВведите 1, 2, 3 или 4 в соответсвии с тем, что вы хотите сделать или 0 чтобы завершить ход:')
    else:
        return('\n' * 100 + '\nМесяцев прошло: ' + str(months) + '\n\nСОСТОЯНИЕ РЫНКА\n' + 'Предложений сырья: ' + str(int(len(players) * metall_offer[market_stade - 1])) + '\nМинимальная цена сырья: ' + str(min_price[market_stade - 1]) + ' т.р.\nСпрос на истребители: ' + str(int(len(players) * spros[market_stade - 1])) + '\nМаксимальная цена истребителя: ' + str(max_price[market_stade - 1]) + ' т.р.\n\nВАШЕ СОСТОЯНИЕ\nСостояние счета — ' + str(human.money) + ' т.р\nКоличество сырья на складе — ' + str(human.metall) + ' ед.\nКоличество готовых истребителей в наличии — ' + str(human.plane) + ' шт.\nКоличество производственных цехов — ' + str(human.workshop) + '\n\nВЗАИМОДЕЙСТВИЕ\n1 - купить сырье у банка\nСЕЙЧАС в планах купить ' + str(human.z_metall[0]) + ' единиц по ' + str(human.z_metall[1]) + ' т.р. за шт.\n2 - продать истребитель банку\nСЕЙЧАС в планах продать ' + str(human.z_sell_istr[0]) + ' истребителей по ' + str(human.z_sell_istr[1]) + ' т.р. за истребитель\n3 - построить цех\n(цена постройки — 5000 т.р.)\nСЕЙЧАС вы хотите построить ' + str(human.z_workshop) + '\n\nПОСТРОЙКА ИСТРЕБИТЕЛЕЙ\nЦена постройки истребителя — 2000 т.р.\n4 - запустить сборку истребиелей\nСЕЙЧАС в планах построить ' + str(human.z_build_istr) + '\n\nВведите 1, 2, 3 или 4 в соответсвии с тем, что вы хотите сделать или 0 чтобы завершить ход:')
      

if __name__ == "__main__":
    app.run(get_ip(), 5000)