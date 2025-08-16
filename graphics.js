// Links Web - Graphics Engine

class Graphics {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.scale = 3.0; // Start with a reasonable scale
        this.center = new Point(canvas.width / 2, canvas.height / 2);
        this.backgroundColor = '#333333';
        this.wheelbtn = true; // For middle-mouse drag panning

        // Mouse state, mimicking the C++ version
        this.mouse = {
            vir: new Point(),    // virtual coordinates
            real: new Point(),   // real screen coordinates
            L: false, C: false, R: false, // Button released this frame
            l: false, c: false, r: false, // Button is currently down
            w: 0,                        // wheel rotation (-1, 0, or 1)
            lastReal: new Point() // For dragging
        };

        this.setupMouseHandlers();
    }

    // Called once per frame to reset single-frame flags
    updateMouseState() {
        this.mouse.L = false;
        this.mouse.C = false;
        this.mouse.R = false;
        this.mouse.w = 0;
    }

    _toScreen(p) {
        return new Point(
            this.center.x + p.x * this.scale,
            this.center.y + p.y * this.scale
        );
    }

    _toVirtual(p) {
         return new Point(
            (p.x - this.center.x) / this.scale,
            (p.y - this.center.y) / this.scale
        );
    }

    setupMouseHandlers() {
        this.canvas.addEventListener('mousemove', (e) => {
            const rect = this.canvas.getBoundingClientRect();
            this.mouse.real.x = e.clientX - rect.left;
            this.mouse.real.y = e.clientY - rect.top;
            this.mouse.vir = this._toVirtual(this.mouse.real);

            // Pan with middle mouse button
            if (this.wheelbtn && this.mouse.c) {
                const diff = this.mouse.real.subtract(this.mouse.lastReal);
                this.center = this.center.add(diff);
            }
            this.mouse.lastReal = this.mouse.real.clone();
        });

        this.canvas.addEventListener('mousedown', (e) => {
            if (e.button === 0) this.mouse.l = true;
            if (e.button === 1) this.mouse.c = true;
            if (e.button === 2) this.mouse.r = true;
        });

        this.canvas.addEventListener('mouseup', (e) => {
            if (e.button === 0) { this.mouse.l = false; this.mouse.L = true; }
            if (e.button === 1) { this.mouse.c = false; this.mouse.C = true; }
            if (e.button === 2) { this.mouse.r = false; this.mouse.R = true; }
        });

        this.canvas.addEventListener('wheel', (e) => {
            e.preventDefault();
            this.mouse.w = e.deltaY > 0 ? -1 : 1;
            // Zoom centered on the mouse cursor
            const oldVir = this.mouse.vir.clone();
            this.scale *= (1 + this.mouse.w * 0.1);
            if (this.scale < 0.1) this.scale = 0.1;
            const newVir = this._toVirtual(this.mouse.real);
            this.center = this.center.add(newVir.subtract(oldVir).multiply(this.scale));
        });

        // Prevent right-click context menu on canvas
        this.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
    }

    clear() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        this.ctx.fillStyle = this.backgroundColor;
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    }

    line(p1, p2, color) {
        const s1 = this._toScreen(p1);
        const s2 = this._toScreen(p2);

        this.ctx.beginPath();
        this.ctx.moveTo(s1.x, s1.y);
        this.ctx.lineTo(s2.x, s2.y);
        this.ctx.strokeStyle = color;
        this.ctx.stroke();
    }

    circle(p, radius, color, filled = false) {
        const s = this._toScreen(p);
        const r = radius * this.scale;

        this.ctx.beginPath();
        this.ctx.arc(s.x, s.y, r, 0, 2 * Math.PI);
        if (filled) {
            this.ctx.fillStyle = color;
            this.ctx.fill();
        } else {
            this.ctx.strokeStyle = color;
            this.ctx.stroke();
        }
    }

    rect(p1, p2, color, filled = false) {
        const s1 = this._toScreen(p1);
        const s2 = this._toScreen(p2);

        if (filled) {
            this.ctx.fillStyle = color;
            this.ctx.fillRect(s1.x, s1.y, s2.x - s1.x, s2.y - s1.y);
        } else {
            this.ctx.strokeStyle = color;
            this.ctx.strokeRect(s1.x, s1.y, s2.x - s1.x, s2.y - s1.y);
        }
    }

    pline(points, color) {
        if (points.length < 2) return;

        this.ctx.beginPath();
        let firstPoint = true;

        for (let i = 0; i < points.length - 1; i++) {
            if (points[i].equals(points[i+1])) {
                // End of a segment in the original code
                this.ctx.strokeStyle = color;
                this.ctx.stroke();
                this.ctx.beginPath();
                firstPoint = true;
                continue;
            }

            if (points[i].heel) {
                 if (firstPoint) {
                    const s = this._toScreen(points[i]);
                    this.ctx.moveTo(s.x, s.y);
                    firstPoint = false;
                }
                const s2 = this._toScreen(points[i+1]);
                this.ctx.lineTo(s2.x, s2.y);
            } else {
                // Not a continuous line, end current path and start new one
                this.ctx.strokeStyle = color;
                this.ctx.stroke();
                this.ctx.beginPath();
                firstPoint = true;
            }
        }
        this.ctx.strokeStyle = color;
        this.ctx.stroke();
    }

    pcircle(cyclos, color) {
        for (const c of cyclos) {
            if (c.r === 0) break;
            this.circle(c.pos, c.r, color, false);
        }
    }
}
