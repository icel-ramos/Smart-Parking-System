const SVG_ICONS = {
  logo: '<svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="#3b82f6" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="18" height="18" rx="4"/><path d="M9 17V7h4a3 3 0 0 1 0 6H9"/></svg>',
  menu: '<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><line x1="4" y1="6" x2="20" y2="6"/><line x1="4" y1="12" x2="20" y2="12"/><line x1="4" y1="18" x2="20" y2="18"/></svg>',
  bell: '<svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M18 8A6 6 0 0 0 6 8c0 7-3 9-3 9h18s-3-2-3-9"/><path d="M13.73 21a2 2 0 0 1-3.46 0"/></svg>',
  user: '<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/><circle cx="12" cy="7" r="4"/></svg>',
  dashboard: '<svg width="17" height="17" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="3" width="7" height="9" rx="1"/><rect x="14" y="3" width="7" height="5" rx="1"/><rect x="14" y="12" width="7" height="9" rx="1"/><rect x="3" y="16" width="7" height="5" rx="1"/></svg>',
  map: '<svg width="17" height="17" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="3 6 9 3 15 6 21 3 21 18 15 21 9 18 3 21"/><line x1="9" y1="3" x2="9" y2="18"/><line x1="15" y1="6" x2="15" y2="21"/></svg>',
  hardware: '<svg width="17" height="17" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="2" width="20" height="8" rx="2"/><rect x="2" y="14" width="20" height="8" rx="2"/><line x1="6" y1="6" x2="6.01" y2="6"/><line x1="6" y1="18" x2="6.01" y2="18"/></svg>',
  billing: '<svg width="17" height="17" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="5" width="20" height="14" rx="2"/><line x1="2" y1="10" x2="22" y2="10"/></svg>',
  compliance: '<svg width="17" height="17" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/><polyline points="9 12 11 14 15 10"/></svg>',
  settings: '<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 2.83-2.83l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>',
  logout: '<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/><polyline points="16 17 21 12 16 7"/><line x1="21" y1="12" x2="9" y2="12"/></svg>',
  barrier: '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M4 20v-8a2 2 0 0 1 2-2h14"/><path d="M4 16h16"/><circle cx="6" cy="13" r="1"/></svg>',
  exit: '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="9 14 4 9 9 4"/><path d="M20 20v-7a4 4 0 0 0-4-4H4"/></svg>',
  lock: '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="11" width="18" height="11" rx="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg>',
  search: '<svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg>',
  file: '<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/></svg>',
  users: '<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"/><circle cx="9" cy="7" r="4"/><path d="M23 21v-2a4 4 0 0 0-3-3.87"/><path d="M16 3.13a4 4 0 0 1 0 7.75"/></svg>',
  info: '<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><line x1="12" y1="16" x2="12" y2="12"/><line x1="12" y1="8" x2="12.01" y2="8"/></svg>',
  upload: '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/></svg>',
  download: '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" y1="15" x2="12" y2="3"/></svg>'
};

window.toggleSidebar = function () {
  const sidebar = document.querySelector('.sidebar');
  if (!sidebar) return;

  let overlay = document.querySelector('.sidebar-overlay');
  if (!overlay) {
    overlay = document.createElement('div');
    overlay.className = 'sidebar-overlay';
    overlay.onclick = window.toggleSidebar;
    document.body.appendChild(overlay);
  }

  sidebar.classList.toggle('open');
  overlay.classList.toggle('open');
};

document.addEventListener('DOMContentLoaded', () => {
  const profileContainer = document.querySelector('.user-profile');
  const bellIcon = document.querySelector('.bell-icon');
  const avatar = document.querySelector('.avatar');

  if (!profileContainer || !bellIcon || !avatar) return;

  // Unread red badge on Bell Icon
  const badge = document.createElement('span');
  badge.className = 'bell-badge';
  badge.id = 'notifBadge';
  bellIcon.appendChild(badge);

  // Notification Dropdown Container
  const notifDropdown = document.createElement('div');
  notifDropdown.className = 'notif-dropdown';
  notifDropdown.innerHTML = `
    <div class="notif-header">
      <strong>Notifications</strong>
      <button type="button" class="notif-clear-btn" id="clearNotifsBtn">Clear all</button>
    </div>
    <div class="notif-list" id="notifList"></div>
  `;
  profileContainer.appendChild(notifDropdown);

  // Account Dropdown Container
  const savedEmail = localStorage.getItem('parkEasyEmail') || 'Not signed in';
  avatar.textContent = savedEmail !== 'Not signed in' ? savedEmail.charAt(0).toUpperCase() : '👤';

  const accountDropdown = document.createElement('div');
  accountDropdown.className = 'account-dropdown';
  accountDropdown.innerHTML = `
    <div class="account-header">
      <div class="account-name" id="menuUserName">System Account</div>
      <div class="account-email" id="menuUserEmail">${savedEmail}</div>
    </div>
    <a href="comply&Setting.html" class="account-item">⚙️ Account &amp; Settings</a>
    <button type="button" class="account-item danger" id="menuLogoutBtn">↩️ Sign Out</button>
  `;
  profileContainer.appendChild(accountDropdown);

  // Render Notifications from Storage
  function renderNotifications() {
    const items = JSON.parse(localStorage.getItem('parkEasyNotifs') || '[]');
    const listEl = document.getElementById('notifList');
    if (!listEl) return;

    if (items.length === 0) {
      listEl.innerHTML = '<div class="notif-empty">No new notifications</div>';
      badge.classList.remove('active');
      badge.textContent = '0';
    } else {
      badge.classList.add('active');
      badge.textContent = items.length;
      listEl.innerHTML = items.map(item => `
        <div class="notif-item">
          <span class="notif-item-title">${item.title}</span>
          <span class="notif-item-sub">${item.message}</span>
          <span class="notif-item-time">${item.time}</span>
        </div>
      `).join('');
    }
  }

  // Global helper to push a notification from any page or Firebase event
  window.pushNotification = function (title, message) {
    const items = JSON.parse(localStorage.getItem('parkEasyNotifs') || '[]');
    const now = new Date();
    const timeStr = `${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}`;
    items.unshift({ title, message, time: timeStr });
    localStorage.setItem('parkEasyNotifs', JSON.stringify(items.slice(0, 20)));
    renderNotifications();
  };

  // Toggle Notification Dropdown
  bellIcon.addEventListener('click', (e) => {
    e.stopPropagation();
    accountDropdown.classList.remove('open');
    notifDropdown.classList.toggle('open');
  });

  // Toggle Account Dropdown
  avatar.addEventListener('click', (e) => {
    e.stopPropagation();
    notifDropdown.classList.remove('open');
    accountDropdown.classList.toggle('open');
  });

  // Clear All Notifications
  document.getElementById('clearNotifsBtn').addEventListener('click', (e) => {
    e.stopPropagation();
    localStorage.removeItem('parkEasyNotifs');
    renderNotifications();
  });

  // Sign Out Handler
  document.getElementById('menuLogoutBtn').addEventListener('click', () => {
    sessionStorage.removeItem('parkEasyLoggedIn');
    window.location.href = 'index.html';
  });

  // Close Dropdowns When Clicking Outside
  document.addEventListener('click', (e) => {
    if (!profileContainer.contains(e.target)) {
      notifDropdown.classList.remove('open');
      accountDropdown.classList.remove('open');
    }
  });

  renderNotifications();
});