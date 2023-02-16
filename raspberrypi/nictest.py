import RPi.GPIO as GPIO
import time

CTR_CLK = 0
CTR_DATA = 0
CLOCK_DELAY = 1 # minimum time to wait in between clock pulses in seconds


pinout = (CTR_DATA, CTR_CLK)

GPIO.setmode(GPIO.BCM)
GPIO.setup(pinout, GPIO.OUT)


clock_state = False
def toggle_clock():

    print("Pulsing clock!")

    if clock_state == False:
        GPIO.output(CTR_CLK, True)
        clock_state = True
    else:
        GPIO.output(CTR_CLK, False)
        clock_state = False

    time.sleep(CLOCK_DELAY)



# This is slow, we probably don't need to change the pin state each time (but it easy!!)
def send_bit(state):

    if (state == 1):
        state = True
    if (state == 0):
        state = False

    print("Sending bit ({})!".format(state))

    GPIO.setup(CTR_DATA, GPIO.OUT)
    GPIO.output(CTR_DATA, state)
    toggle_clock()



# returns bits as booleans true/false
def read_bit():

    GPIO.setup(CTR_DATA, GPIO.IN)
    data = GPIO.input(CTR_DATA)
    print("Read in bit ({})!".format(data))

    toggle_clock()

    return data



def write_network_address(net_addr):

    print("Sending initial start bit (1)...")
    send_bit(1)

    print("Sending opcode 0 (000)...")
    send_bit(0)
    send_bit(0)
    send_bit(0)

    print("Sending new 8 bit network address...")
    bits_left = 8
    while (bits_left > 0):
        output = net_addr & 0x1
        net_addr = net_addr >> 1
        send_bit(output)
        bits_left -= 1

    print("Checking sanity bit...")

    if read_bit():
        print("sanity ok!")
    else:
        print("sanity check failed :(")




def read_network_address():

    print("Sending initial start bit (1)...")
    send_bit(1)

    print("Sending opcode 1 (001)...")
    send_bit(0)
    send_bit(0)
    send_bit(1)

