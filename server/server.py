from flask import Flask, request, render_template_string, jsonify, redirect, url_for
import dutch_database

app = Flask(__name__)

html = """
<!doctype html>
<html>
    <head>
        <title>Words</title>
        <style>
            form {
                padding: 5%;
            }

            input {
                font-size: 150%;
                text-align: center;
                margin: auto;
                width: 100%;
                box-sizing: border-box
            }

            p {
                margin: auto;
                width: 50%;
                padding: 5px 0;
            }

            button {
                font-size: 150%;
                text-align: center;
                margin: auto;
                width: 100%;
            }
        </style>
        </head>
    <body>
    <form method="POST">
        <p><input type="text" name="dutch" placeholder="Dutch" required></p>
        <p><input type="text" name="english" placeholder="English" required></p>
        <p><button type="submit">Submit</button></p>
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

@app.route("/api/success", methods=["GET"])
def success():
    word = request.args.get('word').lower().replace('-', ' ')
    if word:
        dutch_database.add_success(word)
    return render_template_string(html)

@app.route("/api/remove_word", methods={"GET", "POST"})
def remove_word():
    print(request.form)
    word = request.form.get('word').lower().replace('-', ' ')
    if word:
        dutch_database.remove_word(word)
    return redirect(url_for("view_database"))

@app.route("/view", methods={"GET"})
def view_database():    
    view_start = """
    <!doctype html>
    <html>
        <head><title>Words</title></head>
        <body>
            <table>
                <thead>
                    <tr>
                        <th>Dutch</th>
                        <th>English</th>
                        <th>Success Count</th>
                        <th>Remove</th>
                    </tr>
                </thead>
                <tbody>
    """
    view_end = """
                </tbody>
            </table>
        </body>
    </html>
    """

    words = dutch_database.get_all_words()
    for word in words:
        view_start += f"""
                    <tr>
                        <td>{word[0].capitalize()}</td>
                        <td>{word[1].capitalize()}</td>
                        <td>{word[2]}</td>
                        <td>
                            <form action="/api/remove_word" method="POST">
                                <input type="hidden" name="word" value="{word[0]}:{word[1]}">
                                <button type="submit">Delete</button>
                            </form>
                        </td>
                    </tr>
                    """

    return render_template_string(view_start + view_end)

if __name__ == "__main__":
    dutch_database.setup()
    app.run(host="0.0.0.0", port=5000)
