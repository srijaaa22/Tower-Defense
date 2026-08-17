import { useState, useEffect } from 'react'
import './App.css'

function formatTime(totalSeconds) {
  const minutes = Math.floor(totalSeconds / 60)
  const seconds = totalSeconds % 60
  return `${minutes}:${seconds.toString().padStart(2, "0")}`
}

function App() {
  const [scores, setScores] = useState([])

  useEffect(() => {
    fetch('http://localhost:8000/leaderboard')
      .then(r => r.json())
      .then(setScores)
  }, [])

  return (
    <div className="leaderboard">
      <h2>Tower Defense Leaderboard</h2>
      <table>
        <thead>
          <tr>
            <th>#</th>
            <th>Player</th>
            <th>Time Survived</th>
            <th>Won</th>
          </tr>
        </thead>
        <tbody>
          {scores.map((s, i) => (
            <tr key={i}>
              <td>{i + 1}</td>
              <td>{s.player}</td>
              <td>{formatTime(s.score)}</td>
              <td>{s.gameWon ? "Yes": "No"}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )
}

export default App