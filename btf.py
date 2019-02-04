import serial

bt = serial.Serial('/dev/tty.HC-06-SPPDev')
print(bt.name)

message = ""

while 1:
    message = input("Command: ")
    bt.write(bytes(message, encoding = 'utf-8'))


bt.close()