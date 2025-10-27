import socket
import pygame
import sys
import time
import threading

# Configuration
ARDUINO_IP = '192.168.0.101'
PORT = 12345

# Connect to Arduino via TCP
try:
    print(f"Awakening Machine Spirit at {ARDUINO_IP}:{PORT}...")
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((ARDUINO_IP, PORT))
    print("Vox-Link Established: Glory to the Omnissiah.")
except socket.error as e:
    print(f"Awakening failed: {e}")
    print("The Machine Spirit resists your command. Check the connection and try again.")
    sys.exit()

# Pygame controller setup
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("No controller detected. The Omnissiah demands a worthy pilot.")
    sys.exit()

joystick = pygame.joystick.Joystick(0)
joystick.init()
print(f"Using controller: {joystick.get_name()} - The Omnissiah Guides our Holy Command.")

# Detect Joystick Buttons and Axes
def detect_joystick_controls(interactive=False, sample_time=3.0, axis_threshold=0.1, poll_interval=0.01):
    """
    Detect and list all joystick controls.
    - Prints counts and current values for axes, buttons, hats, and balls.
    - If interactive=True, watches for changes for `sample_time` seconds and reports which control changed.
    Returns a dict with counts.
    """
    pygame.event.pump()
    axes = joystick.get_numaxes()
    buttons = joystick.get_numbuttons()
    hats = joystick.get_numhats()
    balls = joystick.get_numballs()

    print(f"Joystick report: {axes} axes, {buttons} buttons, {hats} hats, {balls} balls")
    for i in range(axes):
        val = joystick.get_axis(i)
        print(f"  Axis {i}: value={val:.3f}")
    for i in range(buttons):
        val = joystick.get_button(i)
        print(f"  Button {i}: state={'Pressed' if val else 'Released'}")
    for i in range(hats):
        val = joystick.get_hat(i)
        print(f"  Hat {i}: position={val}")
    for i in range(balls):
        # get_ball returns relative motion since last call; show as a sample
        val = joystick.get_ball(i)
        print(f"  Ball {i}: movement={val}")

    if interactive:
        print(f"Interactive mapping: move/press controls now for {sample_time} seconds...")
        baseline_axes = [joystick.get_axis(i) for i in range(axes)]
        baseline_buttons = [joystick.get_button(i) for i in range(buttons)]
        baseline_hats = [joystick.get_hat(i) for i in range(hats)]

        start = time.time()
        try:
            while time.time() - start < sample_time:
                pygame.event.pump()
                # axes
                for i in range(axes):
                    v = joystick.get_axis(i)
                    if abs(v - baseline_axes[i]) > axis_threshold:
                        print(f"  -> Axis {i} changed: {v:.3f}")
                        baseline_axes[i] = v
                # buttons
                for i in range(buttons):
                    v = joystick.get_button(i)
                    if v != baseline_buttons[i]:
                        print(f"  -> Button {i} {'Pressed' if v else 'Released'}")
                        baseline_buttons[i] = v
                # hats
                for i in range(hats):
                    v = joystick.get_hat(i)
                    if v != baseline_hats[i]:
                        print(f"  -> Hat {i} moved: {v}")
                        baseline_hats[i] = v

                time.sleep(poll_interval)
        except KeyboardInterrupt:
            print("Interactive mapping interrupted by user.")

    return {"axes": axes, "buttons": buttons, "hats": hats, "balls": balls}


def send_command(cmd):
    # send a command over the socket
    try:
        sock.sendall((cmd + '\n').encode())
        print(f"Sent: {cmd}")
    except Exception as e:
        print(f"Cogitator Error: {e}")

def Litany_of_Command(joystick,s):
    # main driving loop
    # joystick = joystick variable
    # s = motor speed
    while True:
        lx_axis=0
        lt_axis=0
        rt_axis=0
        pygame.event.pump()
        

        # Axes Definition
        lx_axis = joystick.get_axis(0)  # Left stick X-axis
        ly_axis = joystick.get_axis(1)  # Left stick Y-axis
        rx_axis = joystick.get_axis(2)  # Right stick X-axis
        ry_axis = joystick.get_axis(3)  # Right stick Y-axis
        lt_axis = joystick.get_axis(4)  # Left trigger
        rt_axis = joystick.get_axis(5)  # Right trigger
        # Buttons Definition
        a_button = joystick.get_button(0)  # A button
        b_button = joystick.get_button(1)  # B button
        x_button = joystick.get_button(2)  # X button
        y_button = joystick.get_button(3)  # Y button
        lb_button = joystick.get_button(4)  # Left bumper
        rb_button = joystick.get_button(5)  # Right bumper
        back_button = joystick.get_button(6)  # Back button
        start_button = joystick.get_button(7)  # Start button
        ls_button = joystick.get_button(8)  # Left stick button
        rs_button = joystick.get_button(9)  # Right stick button
        # Hats Definition
        hat = joystick.get_hat(0)  # D-pad

        #Deadzone
        dz=0.1

        if b_button:
            break


        # Main Drive and Steering
        rt=(rt_axis +1)/2
        lt=(lt_axis +1)/2
        lx=lx_axis
        if abs(lt)<=dz:
            lt_axis=0
        else:
            lt_axis=(abs(lt)-dz)*lt/abs(lt)
        if abs(rt)<=dz:
            rt_axis=0
        else:
            rt_axis=(abs(rt)-dz)*rt/abs(rt)
        if abs(lx)<=dz:
            lx_axis=0
        else:
            lx_axis=(abs(lx)-dz)*lx/abs(lx)

        
        X=(rt_axis)/(1-dz) - (lt_axis)/(1-dz);
        R=lx_axis/(1-dz);
        

        Cx=0; Cr=0;
        ax=abs(X);
        ar=abs(R);
        if ax>0 and ar==0:
            Cx = s
            Cr = 0
        elif ax==0 and ar>0:
            Cx = 0
            Cr = s
        elif ax>0 and ar>0:
            Cx=s*ax/(ax+ar)
            Cr=s*ar/(ax+ar)
        if X>=0:
            Left = int(1500 + Cx*X+ Cr*R)
            Right = int(1500 - Cx*X + Cr*R)
        else:
            Left = int(1500 + Cx*X- Cr*R)
            Right = int(1500 - Cx*X - Cr*R)
        motor_command = f"{Left},{Right}"
        #print(f"{Left},{Right}")
        send_command(motor_command)
        time.sleep(0.05)
        #send_command("1500,1500")
        #print(f"1500,1500")
        #time.sleep(0.05)



Litany_of_Command(joystick,250)
print(f"Mission Complete, Omnissiah Be Praised")





