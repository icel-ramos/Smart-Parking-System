const form = document.querySelector("form");
const emailInput = document.getElementById("email");
const passwordInput = document.getElementById("password");
const eyeIcon = document.querySelector(".eye-icon");
const rememberCheckbox = document.querySelector('.checkbox-label input[type="checkbox"]');
const forgotLink = document.querySelector(".forgot-link");
const submitBtn = document.querySelector(".btn-submit");

// Create an inline error banner dynamically so your HTML stays clean
const errorBanner = document.createElement("div");
errorBanner.className = "error-banner";
errorBanner.hidden = true;
form.parentNode.insertBefore(errorBanner, form);

// 1. Restore remembered email if saved previously
window.addEventListener("DOMContentLoaded", () => {
  const savedEmail = localStorage.getItem("parkEasyEmail");
  if (savedEmail) {
    emailInput.value = savedEmail;
    rememberCheckbox.checked = true;
  }
});

// 2. Toggle password visibility when clicking the eye icon
eyeIcon.addEventListener("click", () => {
  const isPassword = passwordInput.type === "password";
  passwordInput.type = isPassword ? "text" : "password";

  // Swap between Eye-Open and Eye-Off SVG
  eyeIcon.innerHTML = isPassword
    ? `<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
         <path d="M2 12s3-7 10-7 10 7 10 7-3 7-10 7-10-7-10-7Z"></path>
         <circle cx="12" cy="12" r="3"></circle>
       </svg>`
    : `<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
         <path d="M9.88 9.88a3 3 0 1 0 4.24 4.24"></path>
         <path d="M10.73 5.08A10.43 10.43 0 0 1 12 5c7 0 10 7 10 7a13.16 13.16 0 0 1-1.67 2.68"></path>
         <path d="M6.61 6.61A13.526 13.526 0 0 0 2 12s3 7 10 7a9.74 9.74 0 0 0 5.39-1.61"></path>
         <line x1="2" y1="2" x2="22" y2="22"></line>
       </svg>`;
});

// 3. Handle Sign-In Form Submission
form.addEventListener("submit", (e) => {
  e.preventDefault();
  errorBanner.hidden = true;

  const email = emailInput.value.trim();
  const password = passwordInput.value.trim();

  if (!email || !password) {
    errorBanner.textContent = "Please enter both your email address and password.";
    errorBanner.hidden = false;
    return;
  }

  // Save or remove email based on "Remember me"
  if (rememberCheckbox.checked) {
    localStorage.setItem("parkEasyEmail", email);
  } else {
    localStorage.removeItem("parkEasyEmail");
  }

  submitBtn.disabled = true;
  submitBtn.textContent = "Signing in...";

  setTimeout(() => {
    // Demo credentials check (accepts your placeholder email or admin email)
    const validEmails = ["juandelacruz@gmail.com", "alex.m@parkeasy.ph"];
    if (validEmails.includes(email.toLowerCase()) && password === "ParkEasy123") {
      sessionStorage.setItem("parkEasyLoggedIn", "true");
      window.location.href = "Dashboard.html"; // Redirects to your dashboard
    } else {
      errorBanner.innerHTML = 'Invalid login. Try Again';
      errorBanner.hidden = false;
      submitBtn.disabled = false;
      submitBtn.textContent = "Sign In";
    }
  }, 500);
});

// 4. Forgot password handler
forgotLink.addEventListener("click", (e) => {
  e.preventDefault();
  alert("Please contact your Park Easy System Administrator to reset your password.");
});
