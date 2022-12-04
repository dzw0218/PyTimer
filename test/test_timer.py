import MyTimer
import os
import traceback
import time

print("PID: ", os.getpid())

g_tid = -1
startTime = time.time()
def handler(*args, **kwargs):
    print("handler called: ", args, kwargs)
    if time.time() - startTime > 10 and g_tid != -1:
        MyTimer.getTimer().CancelTimer(g_tid)
        print("Timer has been canceled.")


def main_loop():
    timer = MyTimer.getTimer()
    global g_tid
    g_tid = timer.AddTimer(True, 2.0, handler, 1, 2, 3, key1="hello", key2="world")
    try:
        while True:
            try:
                timer.MainLoop()
            except KeyboardInterrupt:
                os._exit(1)
            except:
                traceback.print_exc()
    except KeyboardInterrupt:
        os._exit(1)
    except:
        traceback.print_exc()


if __name__ == '__main__':
    print('Init timer ...')
    main_loop()
