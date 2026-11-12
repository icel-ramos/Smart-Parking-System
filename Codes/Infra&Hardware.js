
    // Gate Logic
    function setGateMode(gate, mode) {
      const autoBtn = document.getElementById(`${gate}-auto`);
      const manualBtn = document.getElementById(`${gate}-manual`);
      const openBtn = document.getElementById(`${gate}-open`);
      const closeBtn = document.getElementById(`${gate}-close`);
      const note = document.getElementById(`${gate}-note`);

      if (mode === 'auto') {
        autoBtn.classList.add('active-blue');
        manualBtn.classList.remove('active-blue');
        openBtn.disabled = true;
        closeBtn.disabled = true;
        openBtn.classList.remove('active-green');
        closeBtn.classList.remove('active-red');
        note.textContent = 'System controlled via auto-loop sensor';
      } else {
        manualBtn.classList.add('active-blue');
        autoBtn.classList.remove('active-blue');
        openBtn.disabled = false;
        closeBtn.disabled = false;
        note.textContent = 'Manual control mode activated';
        const isOpen = document.getElementById(`${gate}-status`).textContent === 'Open';
        setGateState(gate, isOpen ? 'Open' : 'Closed');
      }
    }

    function setGateState(gate, state) {
      const statusBadge = document.getElementById(`${gate}-status`);
      const openBtn = document.getElementById(`${gate}-open`);
      const closeBtn = document.getElementById(`${gate}-close`);

      statusBadge.textContent = state;
      if (state === 'Open') {
        statusBadge.className = 'badge open';
        openBtn.classList.add('active-green');
        closeBtn.classList.remove('active-red');
      } else {
        statusBadge.className = 'badge closed';
        closeBtn.classList.add('active-red');
        openBtn.classList.remove('active-green');
      }
    }

    // Parking Slots Data & Logic
    const slots = [
      { id: 1, status: 'Available', mode: 'auto', light: 'green' },
      { id: 2, status: 'Available', mode: 'auto', light: 'green' },
      { id: 3, status: 'Available', mode: 'auto', light: 'green' },
      { id: 4, status: 'Available', mode: 'auto', light: 'green' },
    ];

    function renderSlots() {
      const container = document.getElementById('slots-container');
      container.innerHTML = slots.map((slot, index) => {
        const isAuto = slot.mode === 'auto';
        const dotColor = slot.status === 'Occupied' ? 'red' : 'green';
        const greenClass = !isAuto && slot.light === 'green' ? 'active-green' : '';
        const redClass = !isAuto && slot.light === 'red' ? 'active-red' : '';

        return `
          <div class="card">
            <div class="card-header">
              <div class="card-title-group">
                <span class="dot ${dotColor}"></span>
                <span>Slot ${slot.id}</span>
              </div>
              <span class="status-text">${slot.status}</span>
            </div>

            <div class="btn-row">
              <button class="pill-btn ${isAuto ? 'active-blue' : ''}" onclick="setSlotMode(${index}, 'auto')">Auto</button>
              <button class="pill-btn ${!isAuto ? 'active-blue' : ''}" onclick="setSlotMode(${index}, 'manual')">Manual</button>
            </div>

            <div class="btn-row">
              <button class="pill-btn ${greenClass}" ${isAuto ? 'disabled' : ''} onclick="setSlotLight(${index}, 'green')">Green Light</button>
              <button class="pill-btn ${redClass}" ${isAuto ? 'disabled' : ''} onclick="setSlotLight(${index}, 'red')">Red Light</button>
            </div>
          </div>
        `;
      }).join('');
    }

    function setSlotMode(index, mode) {
      slots[index].mode = mode;
      renderSlots();
    }

    function setSlotLight(index, color) {
      slots[index].light = color;
      slots[index].status = color === 'green' ? 'Available' : 'Occupied';
      renderSlots();
    }

    // Connected Hardware Devices Data & Logic
    const devices = [
      { id: 'BR-01', type: 'Barrier', name: 'Main Entrance Barrier', layer: 'Level 1 - North', health: 98, status: 'Online' },
      { id: 'BR-02', type: 'Barrier', name: 'Express Exit Gate', layer: 'Level 1 - South', health: 74, status: 'Warning' },
      { id: 'LED-01', type: 'LED', name: 'Smart LED Indicator 1', layer: 'Level 1 - Bay A1', health: 100, status: 'Online' },
      { id: 'LED-02', type: 'LED', name: 'Smart LED Indicator 2', layer: 'Level 1 - Bay A2', health: 100, status: 'Online' },
      { id: 'LED-03', type: 'LED', name: 'Smart LED Indicator 3', layer: 'Level 1 - Bay A3', health: 92, status: 'Online' },
      { id: 'LED-04', type: 'LED', name: 'Smart LED Indicator 4', layer: 'Level 1 - Bay A4', health: 100, status: 'Online' },
      { id: 'SNS-101', type: 'Sensor', name: 'Main ENtrance Sensor', layer: 'Level 1 - North', health: 100, status: 'Online' },
      { id: 'SNS-102', type: 'Sensor', name: 'Main Entrance Sensor', layer: 'Level 1 - South', health: 100, status: 'Online' }
    ];

    function filterDevices() {
      const typeFilter = document.getElementById('typeFilter').value;
      const searchQuery = document.getElementById('searchInput').value.toLowerCase();
      const tbody = document.getElementById('deviceTableBody');

      const filtered = devices.filter(d => {
        const matchesType = typeFilter === 'All' || d.type === typeFilter;
        const matchesSearch =
          d.id.toLowerCase().includes(searchQuery) ||
          d.name.toLowerCase().includes(searchQuery) ||
          d.layer.toLowerCase().includes(searchQuery);
          return matchesType && matchesSearch;
          });
          tbody.innerHTML = filtered.map(d => {
          let colorClass = 'green';
          if (d.health === 0) colorClass = 'red';
          else if (d.health < 80) colorClass = 'yellow';
          const badgeClass = d.status.toLowerCase();
          return `
          ${d.id}
          ${d.status}`;
          }).join('');
          }
          // Initialize UI
          renderSlots();
          filterDevices();