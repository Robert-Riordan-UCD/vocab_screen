from flask import Flask, request, render_template_string, jsonify
import dutch_database

app = Flask(__name__)

html = """
<!doctype html>
<html>
    <head><title>Words</title></head>
    <body>
    <form method="POST">
        <input type="text" name="dutch" placeholder="Dutch" required>
        <input type="text" name="english" placeholder="English" required>
        <button type="submit">Submit</button>
    </form>
    </body>
</html>
"""

@app.route("/", methods=["GET", "POST"])
def index():
    if request.method == "POST":
        dutch_database.add_word(request.form['dutch'], request.form['english'])
    return render_template_string(html)

@app.route("/api/random_word", methods=["GET"])
def random_word():
    word = dutch_database.get_random_word()
    return jsonify({"dutch": word[0], "english": word[1]})

if __name__ == "__main__":
    dutch_database.setup()
    app.run(host="0.0.0.0", port=5000)
