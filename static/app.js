document.addEventListener('DOMContentLoaded', () => {
    const footer = document.querySelector('footer p');
    if (footer) {
        const uptime_start = Date.now();
        setInterval(() => {
            const elapsed = Math.floor((Date.now() - uptime_start) / 1000);
            footer.textContent = `HTTP Server \u00A9 2026 \u2014 Page loaded ${elapsed}s ago`;
        }, 1000);
    }
});
