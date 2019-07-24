

class MyException(Exception):
    pass

try:
    message = input()
except:
    print("Just exited blocking input")
