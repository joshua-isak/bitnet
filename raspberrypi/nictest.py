import RPi.GPIO as GPIO
import time

CTR_CLK = 14
CTR_DATA = 15
CLOCK_DELAY = 0.00001 # minimum time to wait in between clock pulses in seconds


pinout = (CTR_DATA, CTR_CLK)

GPIO.setmode(GPIO.BCM)
GPIO.setup(pinout, GPIO.OUT)


# clock_state = False
def toggle_clock():

    #print("Pulsing clock!")

    if (GPIO.input(CTR_CLK) == False):
        GPIO.output(CTR_CLK, True)
    else:
        GPIO.output(CTR_CLK, False)

    #time.sleep(CLOCK_DELAY)


def set_output():
    GPIO.setup(CTR_DATA, GPIO.OUT)


def set_input():
    GPIO.setup(CTR_DATA, GPIO.IN)

# This is slow, we probably don't need to change the pin state each time (but it easy!!)
def send_bit(state):

    if (state):
        state = True
    if (state == 0):
        state = False

    #print("Sending bit ({})!".format(state))

    # GPIO.setup(CTR_DATA, GPIO.OUT)
    GPIO.output(CTR_DATA, state)
    toggle_clock()



# returns bits as booleans true/false
def read_bit() -> bool:

    # GPIO.setup(CTR_DATA, GPIO.IN)
    data = GPIO.input(CTR_DATA)
    #print("Read in bit ({})!".format(data))

    toggle_clock()

    return data



def write_network_address(net_addr):

    #print("Sending initial start bit (1)...")

    send_bit(1)

    #print("Sending opcode 0 (000)...")
    send_bit(0)
    send_bit(0)
    send_bit(0)

    #print("Sending new 8 bit network address...")
    bits_left = 8
    while (bits_left > 0):
        output = net_addr & 0x80
        net_addr = net_addr << 1
        send_bit(output)
        bits_left -= 1

    set_input()

    #print("Checking sanity bit...")
    sanity = read_bit()

    set_output()

    # if sanity:
    #     print("sanity ok!")
    # else:
    #     print("sanity check failed :(")




def read_network_address():

    #print("Sending initial start bit (1)...")
    send_bit(1)

    #print("Sending opcode 1 (001)...")
    send_bit(0)
    send_bit(0)
    send_bit(1)

    set_input()

    #print("Reading current 8 bit network address...")
    bits_left = 8
    net_addr = 0
    while (bits_left > 0):
        input = read_bit()
        net_addr = net_addr >> 1
        if (input):
            input = 0x80
        net_addr = net_addr | input
        bits_left -= 1

    #print("Checking sanity bit...")
    sanity = read_bit()

    # if sanity:
    #     print("sanity ok!")
    # else:
    #     print("sanity check failed :(")

    set_output()

    return net_addr


# testing
print("-----------------------")
print("writing address 0x07...")
time.sleep(1)
write_network_address(7)

print("reading in network address...")
time.sleep(1)
result = read_network_address()
print("address read in: {}".format(result))

time.sleep(1)



# advanced speed testing

print("-----------------------")
print("testing 255 x 10 read/writes (opcodes 0 & 1)")

start = time.time()

i = 0
x = 0
address = 0
bits_moved = 0
while (x < 10):
    while (i < 255):
        write_network_address(address)
        result = read_network_address()

        if (address != result):
            print("-----------------------")
            print("iteration {}".format(i))
            print("addresses do not match (FAIL)")
            break

        i += 1
        address += 1 # not to exceed 255!
        bits_moved += 26

    x += 1
    address = 0
    i = 0

stop = time.time()
time_elapsed = stop - start
bitrate = bits_moved / time_elapsed

print("//-----------------------//")
print("statistics:")
print("time taken:    {}".format(time_elapsed))
print("bits moved:    {}".format(bits_moved))
print("bitrate (bps): {}".format(bitrate))

GPIO.output(CTR_DATA, False)