import sqlite3, random

success_threshold = 10

def setup():
    db = sqlite3.connect('dutch.db')
    cursor  = db.cursor()
    cursor.execute("""
               CREATE TABLE IF NOT EXISTS translations (
                word TEXT PRIMARY KEY,
                success_count INTEGER
                )
                """)
    db.commit()
    db.close()

def add_word(dutch: str, english: str):
    word = dutch.strip().lower() + ':' + english.strip().lower()

    db = sqlite3.connect('dutch.db')
    cursor = db.cursor()

    cursor.execute("SELECT success_count FROM translations WHERE word = ?", (word,))
    if cursor.fetchone():
        pass
    else:
        cursor.execute("INSERT INTO translations (word, success_count) VALUES (?, 0)", (word,))
    db.commit()
    db.close()

def remove_word(word:str):
    db = sqlite3.connect('dutch.db')
    cursor = db.cursor()

    cursor.execute("DELETE FROM translations WHERE word=?", (word,))
    
    db.commit()
    db.close()
    
def get_random_word():
    db = sqlite3.connect("dutch.db")
    cursor = db.cursor()
    cursor.execute("SELECT word, success_count FROM translations WHERE success_count < ?", (success_threshold,))
    word = random.choice(cursor.fetchall())[0]
    db.close()

    dutch = word.split(':')[0].capitalize()
    english = word.split(':')[1].capitalize()

    return (dutch, english)

def get_all_words():
    db = sqlite3.connect("dutch.db")
    cursor = db.cursor()
    cursor.execute("SELECT word, success_count FROM translations ORDER BY success_count")
    rows = cursor.fetchall()
    db.close()
    words = []
    for row in rows:
        words.append([row[0].split(':')[0], row[0].split(':')[1], row[1]])
    return words

def add_success(word):
    db = sqlite3.connect('dutch.db')
    cursor = db.cursor()

    cursor.execute("UPDATE translations SET success_count = success_count + 1 WHERE word=?", (word,))

    db.commit()
    db.close()

# import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from io import BytesIO
import base64
def make_success_chart():
    # file_name = "success_chart.png"
    
    db = sqlite3.connect("dutch.db")
    cursor = db.cursor()
    cursor.execute("SELECT word, success_count FROM translations",)
    words = cursor.fetchall()
    db.close()

    count = [0 for _ in range(success_threshold+1)]
    
    for word in words:
        count[word[1]] += 1
    
    # https://matplotlib.org/stable/gallery/user_interfaces/web_application_server_sgskip.html
    fig = Figure()
    ax = fig.subplots()
    ax.bar([i for i in range(success_threshold+1)], count)
    ax.set_ylabel('Number of words')
    ax.set_xlabel('Success count')
    
    buf = BytesIO()
    fig.savefig(buf, format='png')
    data = base64.b64encode(buf.getbuffer()).decode('ascii')
    
    return data
