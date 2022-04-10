import os.path

from flask import Flask, abort, send_from_directory, make_response

app = Flask(__name__)


@app.route('/')
def hello_world():  # put application's code here
    return 'Hello World!'


@app.route('/checkUpdate/<item>')
def check_update_firmware(item):
    directory = os.path.join(os.getcwd(), 'json')
    response = make_response(send_from_directory(directory, f'{item}.json', as_attachment=True))
    response.headers['Content-Type'] = 'application/json'
    response.headers.remove('Content-Disposition')
    return response


@app.route('/downloads/<download_dir>/<filename>')
def downloads(download_dir, filename):
    directory = os.path.join(os.getcwd(), 'downloads', download_dir)
    response = make_response(send_from_directory(directory, filename, as_attachment=True))
    response.headers["Content-Disposition"] = "attachment; filename={}".format(filename.encode().decode('latin-1'))
    return response


if __name__ == '__main__':
    app.run()
