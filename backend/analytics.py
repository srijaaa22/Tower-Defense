import pandas as pd
import sqlite3
import matplotlib.pyplot as plt

# --- Scores ---
conn = sqlite3.connect("game.db")
df = pd.read_sql("SELECT * FROM Scores", conn)
conn.close()

print(df[['Score', 'Waves']].describe())

# Chart 1: Score distribution
df['Score'].hist(bins=15, color='#3b82f6', edgecolor='white')
plt.title('Score Distribution')
plt.xlabel('Score')
plt.savefig('Score_dist.png', dpi=150, bbox_inches='tight')
plt.clf()

# Chart 2: Score vs waves
df.plot.scatter(x='Waves', y='Score', color='#22c55e')
plt.title('Score vs Waves Survived')
plt.savefig('Score_vs_Waves.png', dpi=150, bbox_inches='tight')
plt.clf()

# Chart 3: Top 10 players
top10 = df.groupby('Player')['Score'].max().sort_values(ascending=False).head(10)
top10.plot(kind='bar', color='#f59e0b')
plt.title('Top 10 Players')
plt.xticks(rotation=45)
plt.savefig('top10.png', dpi=150, bbox_inches='tight')
plt.clf()

# --- Benchmark ---
bench = pd.read_csv('../bench.csv',
                    names=['frame', 'mode', 'ms', 'entities'])

avg_by_mode = bench.groupby('mode')['ms'].mean()
print(avg_by_mode)

# Chart 4: Naive vs HashGrid
avg_by_mode.plot(kind='bar', color=['#ef4444', '#22c55e'])
plt.title('Avg Frame Time: Naive O(N²) vs Hash Grid')
plt.ylabel('ms / frame')
plt.xticks(rotation=0)
plt.savefig('benchmark.png', dpi=150, bbox_inches='tight')
plt.clf()

print("Charts saved.")