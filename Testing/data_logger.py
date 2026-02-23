import csv
import datetime
import json
import os

class DataLogger:
    """
    CSV data logger for JSON-formatted serial messages.
    - Appends current date to filename
    - Rotates to a new file at midnight
    Example:
        Base filename: "log.csv"
        Output files:  "log_2025_10_14.csv", "log_2025_10_15.csv", ...
    """

    def __init__(self, base_filename):
        self.base_filename = base_filename
        self.current_date = None
        self.filename = None
        self.file = None
        self.writer = None
        self.fieldnames = None

    def _get_dated_filename(self):
        """Generate filename with current date suffix."""
        date_str = datetime.date.today().strftime("%Y_%m_%d")

        # Split filename into base + extension
        base, ext = os.path.splitext(self.base_filename)
        return f"{base}_{date_str}{ext or '.csv'}"

    def _open_new_file(self):
        """Close any open file and start a new one for today."""
        if self.file:
            self.file.close()

        self.filename = self._get_dated_filename()
        self.file = open(self.filename, 'w', newline='', encoding='utf-8')
        self.writer = csv.writer(self.file)
        self.fieldnames = None
        self.current_date = datetime.date.today()
        print(f"[DataLogger] Started new log file: {self.filename}")

    def start(self):
        """Start logging to today's dated file."""
        self._open_new_file()

    def _rotate_if_new_day(self):
        """Check if the date changed and rotate file if needed."""
        today = datetime.date.today()
        if today != self.current_date:
            print("[DataLogger] Midnight detected — rotating log file...")
            self._open_new_file()

    def log(self, message):
        """Parse JSON string and write values to CSV."""
        if not self.writer:
            return

        # Rotate file at midnight
        self._rotate_if_new_day()

        # Try to decode JSON
        try:
            data = json.loads(message)
        except json.JSONDecodeError:
            print(f"[DataLogger] Invalid JSON skipped: {message}")
            return

        # Create header if this is a new file or first message
        if self.fieldnames is None:
            self.fieldnames = list(data.keys())
            header = ["Timestamp"] + self.fieldnames
            self.writer.writerow(header)
            print(f"[DataLogger] CSV header written: {self.fieldnames}")

        # Write timestamp + values
        ts = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
        values = [data.get(k, "") for k in self.fieldnames]
        self.writer.writerow([ts] + values)
        self.file.flush()

    def stop(self):
        """Close the CSV file cleanly."""
        if self.file:
            print(f"[DataLogger] Logging stopped: {self.filename}")
            self.file.close()
            self.file = None
            self.writer = None
