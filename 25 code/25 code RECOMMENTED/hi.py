import socket
import pygame
import time

# Initialize Pygame and the joystick
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("No joystick detected!")
    exit()
joystick = pygame.joystick.Joystick(0)
joystick.init()
print("Joystick detected:", joystick.get_name())

# Arduino IP and port (change to match your Arduino's IP)
arduino_ip = "192.168.0.101"  
port = 12345

# Create a TCP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((arduino_ip, port))
print("Connected to Arduino.")

def map_axis_to_pwm(axis_val):
    # axis_val is in the range [-1, 1]
    # Map to PWM range [1000, 2000] with 1500 as neutral
    pwm = int(1500 + axis_val * 500)
    return pwm

try:
    while True:
        pygame.event.pump()  # Update joystick state
        
        # Using axis 1 for left joystick vertical and axis 3 for right joystick vertical
        left_axis = joystick.get_axis(1)
        right_axis = joystick.get_axis(3)
        
        # Invert the left_axis to fix direction for pins 9 and 10
        left_pwm = map_axis_to_pwm(-left_axis)  # Negative to invert
        right_pwm = map_axis_to_pwm(right_axis)
        
        # Format the message as "left_pwm,right_pwm\n"
        message = f"{left_pwm},{right_pwm}\n"
        print("Sending:", message.strip())
        sock.sendall(message.encode())
        
        time.sleep(0.1)  # Adjust loop speed as necessary

except KeyboardInterrupt:
    print("Exiting...")
finally:
    sock.close()
    pygame.quit()
