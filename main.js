// main.js – leaderboard navigation & rendering
// pages: 0‑today, 1‑week, 2‑all‑time

const periods = ['today', 'week', 'all'];
let currentIndex = 0;

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
  const navUl = document.querySelector('nav ul');
  navUl.innerHTML = '';

  // set flex alignment depending on page
  if (currentIndex === 0) {
    navUl.style.justifyContent = 'flex-end';   // arrow right only
  } else if (currentIndex === 1) {
    navUl.style.justifyContent = 'space-between'; // arrows both sides
  } else {
    navUl.style.justifyContent = 'flex-start'; // arrow left only
  }

  // left arrow
  if (currentIndex > 0) {
    const liLeft = document.createElement('li');
    const aLeft = document.createElement('a');
    aLeft.innerHTML = '&#8592;';
    aLeft.href = '#';
    aLeft.addEventListener('click', () => changePeriod(currentIndex - 1));
    liLeft.appendChild(aLeft);
    navUl.appendChild(liLeft);
  }

  // right arrow
  if (currentIndex < periods.length - 1) {
    const liRight = document.createElement('li');
    const aRight = document.createElement('a');
    aRight.innerHTML = '&#8594;';
    aRight.href = '#';
    aRight.addEventListener('click', () => changePeriod(currentIndex + 1));
    liRight.appendChild(aRight);
    navUl.appendChild(liRight);
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
  return str === 'all' ? 'All Time' : str.charAt(0).toUpperCase() + str.slice(1);
}

function changePeriod(newIndex) {
  currentIndex = newIndex;
  const period = periods[currentIndex];
  document.getElementById('boardTitle').textContent = capitalize(period);
  loadLeaderboard(period);
  renderNav();
}

document.addEventListener('DOMContentLoaded', () => {
  changePeriod(0);
});