document.addEventListener('DOMContentLoaded', () => {
    // Mobile Menu Toggle
    const hamburger = document.querySelector('.hamburger');
    const navLinks = document.querySelector('.nav-links');

    if (hamburger) {
        hamburger.addEventListener('click', () => {
            navLinks.classList.toggle('nav-active');
            // Animate Links (Optional refinement if needed)
        });
    }

    // Smooth scrolling for anchor links
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            e.preventDefault();
            const target = document.querySelector(this.getAttribute('href'));
            if (target) {
                target.scrollIntoView({
                    behavior: 'smooth'
                });
            }
        });
    });

    // Reveal animations on scroll
    const observerOptions = {
        threshold: 0.1
    };

    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('visible');
            }
        });
    }, observerOptions);

    document.querySelectorAll('.section, .project-card, .skill-card').forEach(el => {
        el.classList.add('fade-in-section'); // Add base class for animation
        observer.observe(el);
    });
    // Hero Background Animation
    class HeroAnimation {
        constructor() {
            this.canvas = document.getElementById('hero-canvas');
            if (!this.canvas) return;

            this.ctx = this.canvas.getContext('2d');
            this.width = window.innerWidth;
            this.height = window.innerHeight;
            this.items = [];
            this.mouseX = 0;
            this.mouseY = 0;
            this.targetMouseX = 0;
            this.targetMouseY = 0;

            this.init();
        }

        init() {
            this.resize();
            window.addEventListener('resize', () => this.resize());
            document.addEventListener('mousemove', (e) => this.handleMouseMove(e));

            // Create specific gears to form a "machine"
            this.createGears();
            // Create circuit lines
            this.createCircuits();

            this.animate();
        }

        resize() {
            this.width = window.innerWidth;
            // Limit height to hero section (or viewport if fullscreen)
            const hero = document.querySelector('.hero');
            this.height = hero ? hero.clientHeight : window.innerHeight;
            this.canvas.width = this.width;
            this.canvas.height = this.height;
        }

        handleMouseMove(e) {
            this.targetMouseX = e.clientX;
            this.targetMouseY = e.clientY;
        }

        createGears() {
            // Add some random gears
            for (let i = 0; i < 5; i++) {
                this.items.push({
                    type: 'gear',
                    x: Math.random() * this.width,
                    y: Math.random() * this.height,
                    radius: 30 + Math.random() * 50,
                    teeth: 8 + Math.floor(Math.random() * 8),
                    speed: (Math.random() - 0.5) * 0.02,
                    angle: Math.random() * Math.PI * 2,
                    color: `rgba(56, 189, 248, ${0.05 + Math.random() * 0.1})` // Low opacity accent color
                });
            }
        }

        createCircuits() {
            // Add some circuit nodes/lines
            for (let i = 0; i < 8; i++) {
                this.items.push({
                    type: 'circuit',
                    x: Math.random() * this.width,
                    y: Math.random() * this.height,
                    length: 100 + Math.random() * 200,
                    angle: Math.floor(Math.random() * 4) * (Math.PI / 2), // 0, 90, 180, 270 degrees
                    speed: (Math.random() - 0.5) * 1,
                    offset: Math.random() * 100
                });
            }
        }

        drawGear(ctx, gear) {
            ctx.save();
            ctx.translate(gear.x, gear.y);
            ctx.rotate(gear.angle);
            ctx.fillStyle = gear.color;
            ctx.strokeStyle = gear.color;
            ctx.lineWidth = 2;

            // Draw gear body
            ctx.beginPath();
            const outerRadius = gear.radius;
            const innerRadius = gear.radius * 0.8;
            const holeRadius = gear.radius * 0.3;

            for (let i = 0; i < gear.teeth * 2; i++) {
                const angle = (Math.PI * 2 * i) / (gear.teeth * 2);
                const r = i % 2 === 0 ? outerRadius : innerRadius;
                ctx.lineTo(Math.cos(angle) * r, Math.sin(angle) * r);
            }
            ctx.closePath();
            ctx.fill();

            // Center hole
            ctx.globalCompositeOperation = 'destination-out';
            ctx.beginPath();
            ctx.arc(0, 0, holeRadius, 0, Math.PI * 2);
            ctx.fill();

            // Decorative circle
            ctx.globalCompositeOperation = 'source-over';
            ctx.beginPath();
            ctx.arc(0, 0, holeRadius * 0.5, 0, Math.PI * 2);
            ctx.fillStyle = gear.color;
            ctx.fill();

            ctx.restore();
        }

        drawCircuit(ctx, circuit) {
            ctx.save();
            ctx.translate(circuit.x, circuit.y);
            ctx.rotate(circuit.angle);

            ctx.beginPath();
            ctx.moveTo(0, 0);
            ctx.lineTo(circuit.length, 0);
            ctx.strokeStyle = 'rgba(56, 189, 248, 0.1)';
            ctx.lineWidth = 1;
            ctx.stroke();

            // Moving electron
            const electronPos = (Date.now() / 20 * Math.abs(circuit.speed) + circuit.offset) % circuit.length;
            ctx.beginPath();
            ctx.arc(electronPos, 0, 2, 0, Math.PI * 2);
            ctx.fillStyle = 'rgba(56, 189, 248, 0.5)';
            ctx.fill();

            // End nodes
            ctx.beginPath();
            ctx.arc(0, 0, 3, 0, Math.PI * 2);
            ctx.arc(circuit.length, 0, 3, 0, Math.PI * 2);
            ctx.fillStyle = 'rgba(56, 189, 248, 0.2)';
            ctx.fill();

            ctx.restore();
        }

        animate() {
            this.ctx.clearRect(0, 0, this.width, this.height);

            // Smooth mouse movement for interaction
            this.mouseX += (this.targetMouseX - this.mouseX) * 0.05;
            this.mouseY += (this.targetMouseY - this.mouseY) * 0.05;

            this.items.forEach(item => {
                if (item.type === 'gear') {
                    // Rotate gear based on mouse proximity
                    const dx = this.mouseX - item.x;
                    const dy = this.mouseY - item.y;
                    const dist = Math.sqrt(dx * dx + dy * dy);
                    const interaction = Math.max(0, 1 - dist / 500); // Effect range

                    // Base speed + interaction speed
                    item.angle += item.speed + (interaction * 0.02 * Math.sign(item.speed));
                    this.drawGear(this.ctx, item);
                } else if (item.type === 'circuit') {
                    this.drawCircuit(this.ctx, item);
                }
            });

            requestAnimationFrame(() => this.animate());
        }
    }

    // Initialize animation if canvas exists
    new HeroAnimation();
});

// Tab Switching Logic (Global Scope)
function openTab(evt, tabName) {
    // Hide all tab content
    var i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tab-content");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }

    // Remove active class from all buttons
    tablinks = document.getElementsByClassName("tab-btn");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }

    // Show the specific tab and add active class to button
    document.getElementById(tabName).style.display = "block";
    evt.currentTarget.className += " active";
}

function switchToTab(tabName) {
    // Find the button that opens this specific tab
    const buttons = document.getElementsByClassName('tab-btn');
    for (let btn of buttons) {
        // Check if the onclick attribute contains the tabName
        if (btn.getAttribute('onclick') && btn.getAttribute('onclick').includes(`'${tabName}'`)) {
            btn.click();

            // Scroll to the tabs container for better UX
            const tabsContainer = document.querySelector('.tabs-container');
            if (tabsContainer) {
                tabsContainer.scrollIntoView({ behavior: 'smooth', block: 'start' });
            }
            return;
        }
    }
}
