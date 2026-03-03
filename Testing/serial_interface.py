import serial
import threading
import sys

class SerialInterface:
    def __init__(self, port='/dev/ttyUSB0', baudrate=9600, timeout=1):
        self._ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=timeout
        )
        self._running = False
        self._thread = None
        self._listeners = []

    # ──────── PUBLIC API ────────

    def start(self):
        """Start the serial background reader."""
        if self._running:
            print("Serial reader already running.")
            return
        self._running = True
        self._thread = threading.Thread(target=self._reader_loop, daemon=True)
        self._thread.start()
        print("Serial background reader started.")

    def send(self, data: str):
        """Send data as a line (with newline) over serial."""
        if self._ser.is_open:
            self._ser.write(data.encode('utf-8') + b'\n')
            print(f"[SEND] {data}")
        else:
            print("Serial port is not open.")

    def add_listener(self, callback):
        """Register a callback function to handle incoming messages."""
        if callable(callback):
            self._listeners.append(callback)
            print(f"Listener added: {callback.__name__}")
        else:
            raise ValueError("Callback must be callable")

    def remove_listener(self, callback):
        """Remove a previously registered callback."""
        if callback in self._listeners:
            self._listeners.remove(callback)
            print(f"Listener removed: {callback.__name__}")

    def stop(self):
        """Stop the reader thread and close the serial port."""
        self._running = False
        if self._thread:
            self._thread.join()
        if self._ser.is_open:
            self._ser.close()
        print("Serial interface closed.")

    def is_open(self):
        """Return whether the serial port is open."""
        return self._ser.is_open

    # ──────── INTERNAL (PRIVATE) ────────

    def _reader_loop(self):
        """Continuously read from the serial port and notify listeners."""
        while self._running and self._ser.is_open:
            try:
                line = self._ser.readline().decode('utf-8').strip()
                if line:
                    print(f"[RECV] {line}")
                    self._notify_listeners(line)
            except Exception as e:
                print(f"Read error: {e}")

    def _notify_listeners(self, message):
        """Invoke all listener callbacks with the incoming message."""
        for listener in self._listeners:
            try:
                listener(message)
            except Exception as e:
                print(f"Listener {listener.__name__} error: {e}")


def on_serial_message(msg):
    print(f">> Got serial message: {msg}")

if __name__ == "__main__":
    serial_if = SerialInterface('/dev/ttyACM0', 115200, 0.1)
    #serial_if = SerialInterface('/dev/ttyUSB0', 115200, 0.1)

    try:
        serial_if.add_listener(on_serial_message)
        serial_if.start()

        while True:
            cmd = input("Enter command (or 'exit'): ")
            cmd+='\r\n'
            if cmd.lower() == 'exit':
                break
            serial_if.send(cmd)

    except KeyboardInterrupt:
        print("Interrupted by user.")
    finally:
        serial_if.stop()
