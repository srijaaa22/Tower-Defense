from fastapi import FastAPI
import sqlite3
from pydantic import BaseModel
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

class Score(BaseModel):
    player: str 
    score: int
    waves: int

def init_db():
    con = sqlite3.connect("game.db")
    cursor = con.cursor()
    cursor.execute("CREATE TABLE IF NOT EXISTS Scores (Player TEXT, Score INTEGER, Waves INTEGER)")
    con.commit()
    con.close()

init_db()

@app.post("/score")
def insert_score(s: Score):
    con = sqlite3.connect("game.db")
    cursor = con.cursor()
    cursor.execute("INSERT INTO Scores (Player, Score, Waves) VALUES (?, ?, ?)", (s.player, s.score, s.waves))
    con.commit()
    con.close()
    return {"ok": True}

@app.get("/leaderboard")
def leaderboard():
    con = sqlite3.connect("game.db")
    cursor = con.cursor()
    cursor.execute("SELECT player,score,waves from Scores ORDER BY score DESC LIMIT 10")
    rows = cursor.fetchall()
    con.close()
    return [{"player": r[0], "score": r[1], "waves": r[2]} for r in rows]

