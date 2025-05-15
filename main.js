// main.js – leaderboard navigation & rendering

const periods = ['today', 'week', 'all'];
let currentIndex = 0; // 0‑today, 1‑week, 2‑all‑time

function buildRow(rank, name, score) {
  const row = document.createElement('div');
  row.className = 'grid-row';
  row.innerHTML = `
    <div class="grid-column" id="rank"><h3>${rank}</h3></div>
    <div class="grid-column" id="name"><p>${name}</p></div>
    <div class="grid-column" id="score"><p>${score.toFixed(1)} pts.</p></div>`;
  return row;
}

function renderNav() {
  const nav = document.querySelector('nav ul');
  nav.innerHTML = '';
  // left arrow (index > 0)
  if (currentIndex > 0) {
    const liLeft = document.createElement('li');
    const aLeft = document.createElement('a');
    aLeft.innerHTML = '&#8592;'; // left arrow
    aLeft.href = 'javascript:void(0)';
    aLeft.addEventListener('click', () => changePeriod(currentIndex - 1));
    liLeft.appendChild(aLeft);
    nav.appendChild(liLeft);
  }
  // right arrow (index < last)
  if (currentIndex < periods.length - 1) {
    const liRight = document.createElement('li');
    const aRight = document.createElement('a');
    aRight.innerHTML = '&#8594;'; // right arrow
    aRight.href = 'javascript:void(0)';
    aRight.addEventListener('click', () => changePeriod(currentIndex + 1));
    liRight.appendChild(aRight);
    nav.appendChild(liRight);
  }
}

function loadLeaderboard(period) {
  fetch(`etl/unload.php?period=${period}`)
    .then((r) => r.json())
    .then((data) => {
      const grid = document.getElementById('leaderboardGrid');
      grid.innerHTML = '';
      data.forEach((row) => grid.appendChild(buildRow(row.rank, row.device_id, row.total)));
    })
    .catch((err) => console.error('Leaderboard fetch error', err));
}

function capitalize(str) {
  return str.replace(/^./, (c) => c.toUpperCase());
}

function changePeriod(newIndex) {
  currentIndex = newIndex;
  const period = periods[currentIndex];
  document.getElementById('boardTitle').textContent = capitalize(period === 'all' ? 'all time' : period);
  loadLeaderboard(period);
  renderNav();
}

document.addEventListener('DOMContentLoaded', () => {
  changePeriod(0); // start with Today
});