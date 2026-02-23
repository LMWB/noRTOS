'''
install python can library with 
 - pip install python-can
 - pip install typing_extensions --upgrade

start tha PEAK CAN Dongle on OS via terminal
>>> sudo ip link set can0 up type can bitrate 500000 fd off sample-point 0.4

LED on USB dongle needs to blink green

check socket connection working
>>> ifconfig

needs to show something like this:

can0: flags=193<UP,RUNNING,NOARP>  mtu 16
        unspec 00-00-00-00-00-00-00-00-00-00-00-00-00-00-00-00  txqueuelen 10  (UNSPEC)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 6  bytes 48 (48.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

shut down / restart PEAK CAN Dongle on OS via terminal (LED red)
>>> sudo ip link set can0 down

To display in real-time the list of messages received on the bus, use the candump utility: 
>>> candump can0

should return something like this on receive:
can0  123   [8] 11 22 33 44 55 66 77 88

or use whireshark
Wireshark supports socketcan and can be used to debug python-can messages. Fire it up and watch your new interface.
https://python-can.readthedocs.io/en/stable/interfaces/socketcan.html#wireshark


'''

import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import can

TITLE = "CAN GUI with Signals"

# Predefined CAN IDs for sending
# This is also your base CAN IDs from devices you have physically on the bench
# change it according to your start up sequence on terminal
predefined_can_IDs = ["19003F", "290015", "5004E"] #"19003F"

# dummy payload from 1 .. 8 Bytes
predefined_can_payload = ["01", "01 02", "01 02 03", "01 02 03 04", "01 02 03 04 05", "01 02 03 04 05 06", "01 02 03 04 05 06 07", "01 02 03 04 05 06 07 08"]

# predefined CAN message offsets
CAN_ID_SIGNALS_BASE         = 0x1000
CAN_ID_OFFSET_PRESSURE      = 0x01
CAN_ID_OFFSET_TEMPERATURE   = 0x02
CAN_ID_OFFSET_HUMIDITY      = 0x03
CAN_ID_OFFSET_XXX           = 0x04

# Max log lines
MAX_LOG_LINES = 100

class CanGuiApp:
    def __init__(self, master):
        self.master = master
        master.title(TITLE)

        # CAN Bus setup
        try:
            self.bus = can.Bus(interface='socketcan', channel='can0')
        except Exception as e:
            messagebox.showerror("CAN Error", f"Failed to connect to CAN: {e}")
            master.destroy()
            return

        # --- Signal Monitor ---
        frame_signals = tk.LabelFrame(master, text="Signal Monitor")
        frame_signals.grid(row=0, column=0, columnspan=3, padx=5, pady=5, sticky="ew")

        # Digital signals (6 LED-style indicators)
        tk.Label(frame_signals, text="Digital Signals").grid(row=0, column=0, sticky="w")
        self.digital_labels = []
        for i in range(6):
            lbl = tk.Label(frame_signals, text=f"D{i+1}", width=6, relief="groove", bg="gray")
            lbl.grid(row=1, column=i, padx=2, pady=2)
            self.digital_labels.append(lbl)

        # Analog signals (5 labels)
        tk.Label(frame_signals, text="Analog Signals").grid(row=2, column=0, sticky="w")
        self.analog_labels = []
        for i in range(3):
            lbl = tk.Label(frame_signals, text=f"A{i+1}: ---")
            lbl.grid(row=3 + i//3, column=i % 3, padx=5, pady=2, sticky="w")
            self.analog_labels.append(lbl)
        
        # --- Listener Controls ---
        frame_listen = tk.LabelFrame(master, text="Listen CAN Frame")
        frame_listen.grid(row=1, column=0, columnspan=3, padx=5, pady=5, sticky="ew")
        
        tk.Label(frame_listen, text="CAN ID (hex):").grid(row=0, column=0, sticky='e')
        self.id_listen = ttk.Combobox(frame_listen, values=predefined_can_IDs)
        self.id_listen.set(predefined_can_IDs[0])
        self.id_listen.grid(row=0, column=1)

        # --- Sender Controls ---
        frame_send = tk.LabelFrame(master, text="Send CAN Frame")
        frame_send.grid(row=2, column=0, columnspan=3, padx=5, pady=5, sticky="ew")

        tk.Label(frame_send, text="CAN ID (hex):").grid(row=0, column=0, sticky='e')
        self.id_combo = ttk.Combobox(frame_send, values=predefined_can_IDs)
        self.id_combo.set(predefined_can_IDs[0])
        self.id_combo.grid(row=0, column=1)

        self.extended_id = tk.BooleanVar(value=True)
        tk.Checkbutton(frame_send, text="Extended ID", variable=self.extended_id).grid(row=0, column=2)

        tk.Label(frame_send, text="Payload:").grid(row=1, column=0, sticky='e')
        self.data_combo = ttk.Combobox(frame_send, values=predefined_can_payload)
        self.data_combo.set(predefined_can_payload[0])
        self.data_combo.grid(row=1, column=1, columnspan=2, sticky="ew")

        self.send_button = tk.Button(frame_send, text="Send", command=self.send_frame)
        self.send_button.grid(row=2, column=0, columnspan=3, pady=5)

        # --- Log ---
        frame_log = tk.LabelFrame(master, text="Log")
        frame_log.grid(row=3, column=0, columnspan=3, padx=5, pady=5)

        self.recv_text = scrolledtext.ScrolledText(frame_log, width=60, height=18, state='disabled')
        self.recv_text.grid(row=0, column=0, columnspan=3)

        self.clear_button = tk.Button(frame_log, text="Clear Log", command=self.clear_log)
        self.clear_button.grid(row=1, column=0, columnspan=3, pady=5)

        # Listener
        self.listener = GuiListener(self)
        self.notifier = can.Notifier(self.bus, [self.listener])

        # Shutdown
        master.protocol("WM_DELETE_WINDOW", self.on_close)

    def send_frame(self):
        try:
            can_id = int(self.id_combo.get(), 16)
            data_str = self.data_combo.get().strip()
            data = [int(b, 16) for b in data_str.split()]
            if len(data) > 8:
                raise ValueError("Max 8 data bytes allowed.")

            msg = can.Message(arbitration_id=can_id,
                              data=data,
                              is_extended_id=self.extended_id.get())
            self.bus.send(msg)
            self.log_message(f"[SENT] ID: {can_id:#010x} Data: {msg.data.hex()}")
        except ValueError as ve:
            messagebox.showerror("Input Error", str(ve))
        except can.CanError:
            messagebox.showerror("CAN Error", "Failed to send CAN message")

    def log_message(self, text):
        self.recv_text.configure(state='normal')
        self.recv_text.insert(tk.END, text + '\n')

        # Auto-truncate log
        lines = self.recv_text.get("1.0", tk.END).splitlines()
        if len(lines) > MAX_LOG_LINES:
            self.recv_text.delete("1.0", f"{len(lines) - MAX_LOG_LINES + 1}.0")

        self.recv_text.configure(state='disabled')
        self.recv_text.see(tk.END)

    def clear_log(self):
        self.recv_text.configure(state='normal')
        self.recv_text.delete('1.0', tk.END)
        self.recv_text.configure(state='disabled')

    def update_signals(self, msg):
        base_id = int(self.id_listen.get(), 16)
        PRESSURE_ID = base_id + CAN_ID_SIGNALS_BASE + CAN_ID_OFFSET_PRESSURE
        TEMPERATURE_ID = base_id + CAN_ID_SIGNALS_BASE + CAN_ID_OFFSET_TEMPERATURE
        HUMIDITY_ID = base_id + CAN_ID_SIGNALS_BASE + CAN_ID_OFFSET_HUMIDITY

        if msg.arbitration_id == PRESSURE_ID:
            if len(msg.data) >= 4:
                raw = int.from_bytes(msg.data[0:4], byteorder='big', signed=True)
                press = raw / 100.0
                self.analog_labels[0].config(text=f"Press: {press:.2f} hP")

        elif msg.arbitration_id == TEMPERATURE_ID:
            if len(msg.data) >= 4:
                raw = int.from_bytes(msg.data[0:4], byteorder='big', signed=True)
                temp = raw / 100.0
                self.analog_labels[1].config(text=f"Temp: {temp:.2f} °C")
            
                # a1 = int.from_bytes(msg.data[0:2], byteorder='big', signed=True)
                # a2 = int.from_bytes(msg.data[2:4], byteorder='big', signed=True)
                # a3 = int.from_bytes(msg.data[4:6], byteorder='big', signed=True)
                # a4 = int.from_bytes(msg.data[6:8], byteorder='big', signed=True)
                
        elif msg.arbitration_id == HUMIDITY_ID:
            if len(msg.data) >= 4:
                raw = int.from_bytes(msg.data[0:4], byteorder='big', signed=True)
                hum = raw / 10.0
                self.analog_labels[2].config(text=f"H: {hum:.2f} %")

    def on_close(self):
        self.notifier.stop()
        self.bus.shutdown()
        self.master.destroy()

class GuiListener(can.Listener):
    def __init__(self, gui_app):
        self.gui_app = gui_app

    def on_message_received(self, msg):
        id_fmt = f"{msg.arbitration_id:#010x}" if msg.is_extended_id else f"{msg.arbitration_id:#04x}"
        data_fmt = msg.data.hex()
        text = f"[RECEIVED] ID: {id_fmt} Data: {data_fmt}"
        self.gui_app.master.after(0, self.gui_app.log_message, text)
        self.gui_app.master.after(0, self.gui_app.update_signals, msg)

def main():
    root = tk.Tk()
    app = CanGuiApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()
