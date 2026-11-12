// 1. Parking Rates Data (2 Wheelers defaults to editing mode like screenshot)
const ratesData = [
  { id: "2w", type: "2 Wheelers", firstHour: "20.00", extraHour: "10.00", dailyMax: "100.00", overnight: "80.00", isEditing: false },
  { id: "4w", type: "4 Wheelers", firstHour: "40.00", extraHour: "20.00", dailyMax: "200.00", overnight: "150.00", isEditing: false }
];

// 2. Transactions Data
const transactionsData = [
  { id: "PK-0186", plate: "NCL 4821", vehicle: "4 Wheeler", timeIn: "12 Nov, 09:24 AM", timeOut: "12 Nov, 11:42 AM", amount: "80.00", status: "Paid" },
  { id: "PK-0185", plate: "542 QRT", vehicle: "2 Wheeler", timeIn: "12 Nov, 09:18 AM", timeOut: "12 Nov, 10:50 AM", amount: "30.00", status: "Paid" },
  { id: "PK-0184", plate: "DBV 9036", vehicle: "4 Wheeler", timeIn: "12 Nov, 08:56 AM", timeOut: "—", amount: "100.00", status: "Unpaid" },
  { id: "PK-0183", plate: "NAG 1279", vehicle: "4 Wheeler", timeIn: "12 Nov, 08:40 AM", timeOut: "12 Nov, 10:12 AM", amount: "60.00", status: "Paid" },
  { id: "PK-0182", plate: "781 MKL", vehicle: "2 Wheeler", timeIn: "12 Nov, 08:32 AM", timeOut: "—", amount: "40.00", status: "Unpaid" },
  { id: "PK-0181", plate: "ABC 1234", vehicle: "4 Wheeler", timeIn: "12 Nov, 08:15 AM", timeOut: "12 Nov, 09:10 AM", amount: "40.00", status: "Paid" },
  { id: "PK-0180", plate: "998 XYZ", vehicle: "2 Wheeler", timeIn: "12 Nov, 07:50 AM", timeOut: "12 Nov, 09:45 AM", amount: "30.00", status: "Paid" }
];

let currentPage = 1;
const rowsPerPage = 5;
let filteredTransactions = [...transactionsData];

// Render Parking Rates Table
function renderRates() {
  const tbody = document.getElementById("ratesBody");
  tbody.innerHTML = "";

  ratesData.forEach((rate, index) => {
    const tr = document.createElement("tr");

    if (rate.isEditing) {
      tr.innerHTML = `
        <td>${rate.type}</td>
        <td><div class="rate-input-wrap"><span>&#8369;</span><input type="text" id="first-${index}" value="${rate.firstHour}"></div></td>
        <td><div class="rate-input-wrap"><span>&#8369;</span><input type="text" id="extra-${index}" value="${rate.extraHour}"></div></td>
        <td><div class="rate-input-wrap"><span>&#8369;</span><input type="text" id="daily-${index}" value="${rate.dailyMax}"></div></td>
        <td><div class="rate-input-wrap"><span>&#8369;</span><input type="text" id="night-${index}" value="${rate.overnight}"></div></td>
        <td>
          <div class="action-btns">
            <button class="btn-edit" disabled>Edit</button>
            <button class="btn-save" onclick="saveRate(${index})">Save</button>
          </div>
        </td>
      `;
    } else {
      tr.innerHTML = `
        <td>${rate.type}</td>
        <td class="rate-text">&#8369;${rate.firstHour}</td>
        <td class="rate-text">&#8369;${rate.extraHour}</td>
        <td class="rate-text">&#8369;${rate.dailyMax}</td>
        <td class="rate-text">&#8369;${rate.overnight}</td>
        <td>
          <div class="action-btns">
            <button class="btn-edit" onclick="editRate(${index})">Edit</button>
            <button class="btn-save" disabled>Save</button>
          </div>
        </td>
      `;
    }

    tbody.appendChild(tr);
  });
}

function editRate(index) {
  ratesData[index].isEditing = true;
  renderRates();
}

function saveRate(index) {
  const firstVal = parseFloat(document.getElementById(`first-${index}`).value) || 0;
  const extraVal = parseFloat(document.getElementById(`extra-${index}`).value) || 0;
  const dailyVal = parseFloat(document.getElementById(`daily-${index}`).value) || 0;
  const nightVal = parseFloat(document.getElementById(`night-${index}`).value) || 0;

  ratesData[index].firstHour = firstVal.toFixed(2);
  ratesData[index].extraHour = extraVal.toFixed(2);
  ratesData[index].dailyMax = dailyVal.toFixed(2);
  ratesData[index].overnight = nightVal.toFixed(2);
  ratesData[index].isEditing = false;

  renderRates();
}

// Render Transactions Table & Pagination
function renderTransactions() {
  const tbody = document.getElementById("txBody");
  tbody.innerHTML = "";

  const start = (currentPage - 1) * rowsPerPage;
  const end = start + rowsPerPage;
  const pageItems = filteredTransactions.slice(start, end);

  pageItems.forEach(tx => {
    const tr = document.createElement("tr");
    const badgeClass = tx.status === "Paid" ? "badge-paid" : "badge-unpaid";

    tr.innerHTML = `
      <td class="tx-id">${tx.id}</td>
      <td>${tx.plate}</td>
      <td>${tx.vehicle}</td>
      <td>${tx.timeIn}</td>
      <td>${tx.timeOut}</td>
      <td>&#8369;${tx.amount}</td>
      <td><span class="badge ${badgeClass}">${tx.status}</span></td>
    `;
    tbody.appendChild(tr);
  });

  // Update footer counter
  const totalDisplay = filteredTransactions.length === transactionsData.length ? 186 : filteredTransactions.length;
  const showStart = filteredTransactions.length === 0 ? 0 : start + 1;
  const showEnd = Math.min(end, filteredTransactions.length);
  document.getElementById("showingText").textContent = `Showing ${showStart}–${showEnd} of ${totalDisplay} transactions`;

  renderPagination();
}

function renderPagination() {
  const container = document.getElementById("paginationControls");
  container.innerHTML = `
    <button class="page-btn" onclick="changePage(${currentPage - 1})">&lsaquo;</button>
    <button class="page-btn ${currentPage === 1 ? 'active' : ''}" onclick="changePage(1)">1</button>
    <button class="page-btn ${currentPage === 2 ? 'active' : ''}" onclick="changePage(2)">2</button>
    <button class="page-btn">3</button>
    <span>...</span>
    <button class="page-btn">38</button>
    <button class="page-btn" onclick="changePage(${currentPage + 1})">&rsaquo;</button>
  `;
}

function changePage(page) {
  const maxPage = Math.ceil(filteredTransactions.length / rowsPerPage);
  if (page < 1 || page > maxPage) return;
  currentPage = page;
  renderTransactions();
}

// Search & Filter
function filterTransactions() {
  const query = document.getElementById("searchInput").value.toLowerCase();
  const statusFilter = document.getElementById("txDateFilter").value;

  filteredTransactions = transactionsData.filter(tx => {
    const matchesText = tx.id.toLowerCase().includes(query) || tx.plate.toLowerCase().includes(query);
    const matchesStatus = statusFilter === "all" || tx.status === statusFilter;
    return matchesText && matchesStatus;
  });

  currentPage = 1;
  renderTransactions();
}

document.getElementById("txDateFilter").addEventListener("change", filterTransactions);

// Export to CSV
function exportCSV() {
  const headers = ["Ticket ID,Plate No.,Vehicle,Time In,Time Out,Amount,Status"];
  const rows = filteredTransactions.map(tx => 
    `${tx.id},"${tx.plate}",${tx.vehicle},"${tx.timeIn}","${tx.timeOut}",${tx.amount},${tx.status}`
  );
  
  const csvContent = "data:text/csv;charset=utf-8," + [headers, ...rows].join("\n");
  const encodedUri = encodeURI(csvContent);
  const link = document.createElement("a");
  link.setAttribute("href", encodedUri);
  link.setAttribute("download", "parking_transactions_12Nov2026.csv");
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
}

// KPI Period Switcher
function updateKPIs() {
  const period = document.getElementById("periodFilter").value;
  if (period === "yesterday") {
    document.getElementById("kpiToday").innerHTML = "&#8369;11,530";
    document.getElementById("kpiTodaySub").textContent = "↑ 4.1% vs previous day";
  } else if (period === "week") {
    document.getElementById("kpiToday").innerHTML = "&#8369;78,940";
    document.getElementById("kpiTodaySub").textContent = "↑ 11.0% vs last week";
  } else {
    document.getElementById("kpiToday").innerHTML = "&#8369;12,480";
    document.getElementById("kpiTodaySub").textContent = "↑ 8.2% vs yesterday";
  }
}

// Initial Render
renderRates();
renderTransactions();
