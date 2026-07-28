import { useState, useEffect } from 'react'
import './App.css'

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
            <th>Score</th>
            <th>Waves</th>
          </tr>
        </thead>
        <tbody>
          {scores.map((s, i) => (
            <tr key={i}>
              <td>{i + 1}</td>
              <td>{s.player}</td>
              <td>{s.score}</td>
              <td>{s.waves}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )
}

export default App