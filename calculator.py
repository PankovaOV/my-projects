import math
class Stack():
    def __init__(self):
        self.stack = []

    def get_stack(self):
        return len(self.stack)

    def push(self, elem):
        self.stack.append(elem)

    def pop(self):
        try:
            return self.stack.pop(-1)
        except:
            return None
    def clearx(self):
        self.stack.clear()

    def last(self):
        try:
            return self.stack[-1]
        except:
            return None
def is_number(x):
      try:
          float(x)
          return True
      except ValueError:
          return False
a = str(input("Ну привет, дружок! Хочешь поиграть? Тогда вводи свой пример: "))
a = a.replace(',', '.')
a = list(a)
def lex(a):
  ch_br = True
  
  if len(a) == 0:
      ch_br = False
      print("Ты че, аферист??? Ну, тогда лови ошибку")#проверка на ввод
  else:
      pass
  
  b = ['+', '-', '*', '/', '^', 'sin', 'cos', 'tg', 'ctg', 'ln', 'exp']
  scob = ['+', '-', '*', '/', '^', 'sin', 'cos', 'tg', 'ctg', 'ln', 'exp', '(', ')']
  
  q = Stack()
  w = Stack()

  brackets = []
  for el in a:
      if el == '(' or el == ')':
          brackets.append(el)
  for j in range(len(a)):
      if a[j] == '(' and a[j+1] == ')':
          ch_br = False
          print("Вставь что нибудь в скобки, клоун")
          exit()
      
  if a[len(a) - 1] in b:
      ch_br = False
      print("Ты бредишь??? После знака надо ставить цифру или число, дурак!")#проверка на повтор знаков
  for i in range(len(a) - 1):
      if a[i] in b and a[i+1] in b:
          ch_br = False
          print("Дебил, не можешь знаки нормально расставить!?" )
          exit()
      
  
  if len(brackets) > 0:# проверка на скобки
      for c in brackets:
          if c == '(':
              w.push(c)
          else:
              if w.get_stack() == 0:
                  ch_br = False
                  print(("Идиот, иди скобки перепроверь!"))
                  exit()
              w.pop()
      if w.get_stack() != 0:
          ch_br = False
      else:
          ch_br = True
  
  lst = []
  if ch_br == True:
      for elem in a:
          if elem not in scob:
              q.push(elem)
          elif elem in scob:
              st = ''
              for i in q.stack:
                  st += i
              lst.append(st)
              lst.append(elem)
              q.clearx()
  
  
  st1 = ''
  for j in q.stack:
      st1 += j
  lst.append(st1)
  for m in lst:
      if m == '':
          lst.remove(m)
  if lst[0] == '-':
    lst[0] = 'UM'
  for i in range(1, len(lst)):
    if lst[i] == '-' and lst[i-1] == '(':
      lst[i] = 'UM'
  return lst
def pol(lst):
  r = Stack()
  operr = ['sin', 'cos', 'tg', 'ctg', 'exp', 'ln']
  priority = {'(': 0, ')': 0, 'sin': 1, 'cos': 1, 'tg': 1, 'ctg': 1, 'ln': 1, 'exp': 1, '+': 2, '-': 2, '^': 4, '*': 3, '/': 3}
  queue = []
  
  #функция на определение, число ли это
  for elem in lst: #польская запись
      if elem == "UM":
          r.push("UM")
      elif is_number(elem):
          queue.append(elem)
          if r.last() == "UM":
              queue.append(r.pop())
      else:
          if r.get_stack() == 0:
              r.push(elem)
          else:
              if priority.setdefault(elem) > priority.setdefault(r.last()):
                  r.push(elem)
              elif elem in operr:
                  r.push(elem)
              elif elem == '(':
                  r.push(elem)
              elif elem == ')':
                  while True:
                      if r.last() == '(':
                          r.pop()
                          break
                      else:
                          queue.append(r.pop())
                  if r.last() in operr:
                      queue.append(r.pop())
  
              else:
                  while r.get_stack() != 0 and priority.setdefault(elem) <= priority.setdefault(r.last()):
  
                      queue.append(r.pop())
                  r.push(elem)
  while r.get_stack() != 0:
      queue.append(r.pop())
  return queue

def calc(queue):
  e = Stack()
  for i in queue:
    if is_number(i):
        e.push(i)
    else:
        if i == '+':
            a = float(e.pop())
            b = float(e.pop())
            ab = a + b
            e.push(ab)
        elif i =='-':
            a = float(e.pop())
            b = float(e.pop())
            ab = b - a
            e.push(ab)
        elif i == '*':
            a = float(e.pop())
            b = float(e.pop())
            ab = a * b
            e.push(ab)
        elif i == '/':
            a = float(e.pop())
            b = float(e.pop())
            ab = b / a
            e.push(ab)
        elif i == '^':
            a = float(e.pop())
            b = float(e.pop())
            ab = b ** a
            e.push(ab)
        elif i == 'sin':
            c = float(e.pop())
            d = math.sin(c)
            e.push(d)
        elif i == 'cos':
            c = float(e.pop())
            d = math.cos(c)
            e.push(d)
        elif i == 'tg':
            c = float(e.pop())
            d = math.tan(c)
            e.push(d)
        elif i == 'ctg':
            c = float(e.pop())
            d = 1 / math.tan(c)
            e.push(d)
        elif i == 'exp':
            c = float(e.pop())
            d = math.exp(c)
            e.push(d)
        elif i == 'ln':
            c = float(e.pop())
            d = math.log(c)
            e.push(d)
        elif i == 'UM':
            c = float(e.pop())
            d = (-1) * c
            e.push(d)
  return e.pop() 
def f(y, st):
    b1 = list(st)
    for i in range(len(b1)):
        if b1[i] == 'x' and b1[i-1] != 'e':
          if int(y) < 0:
            s = '(' + str(y) + ')'
            b1[i] = s
          else:
            b1[i] = str(y)
    b1 = ''.join(b1)
    return calc(pol(lex(b1)))
def corni(st):
  eps = 0.00001
  a = float(input("введите нижнюю границу"))
  b = float(input("введите верхнюю границу"))
  if f(a, st) * f(b, st) > 0:
    print("Нет корней")
  else:
    while True:
      c = ((f(a, st) * b) - f(b, st) * a) / (f(a, st) - f(b, st))
      if f(c, st) * f(b, st) > 0:
        b = c
      elif f(c, st) * f(a, st) > 0:
        a = c
      if abs(f(a, st)) <= eps:
        print(a)
        exit()
      elif abs(f(b, st)) <= eps:
        print(a)
        exit()

def integ(st):
  a = float(input("введите нижнюю границу"))
  b = float(input("введите верхнюю границу"))
  h = 10000
  dx = (b - a) / h
  S = 0
  for i in range(h):
      x = a + dx * i
      x1 = x + dx
      S += (((x1 - x) / 6) * ((f(x, st)) + 4 * f((x + x1) / 2, st) + f(x1, st)))
  return S
v = lex(a)
if not 'x' in v:
  print(calc(pol(lex(a))))
else:
  s = input("Хотите найти корни на промежутке или интеграл. Напишите корни или интеграл:\n")
  if s == 'корни':
    print(corni(a))
  elif s == 'интеграл':
    print(integ(a))
    