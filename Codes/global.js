// 1. Collapsible Sidebar Drawer Toggle
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

// 2. Initialize Top-Right Notification Bell & Account Menu on Load
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