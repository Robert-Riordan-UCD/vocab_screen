import sqlite3, random

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

def get_random_word():
    db = sqlite3.connect("dutch.db")
    cursor = db.cursor()
    cursor.execute("SELECT word, success_count FROM translations")
    word = random.choice(cursor.fetchall())[0]
    db.close()

    dutch = word.split(':')[0].capitalize()
    english = word.split(':')[1].capitalize()

    return (dutch, english)
