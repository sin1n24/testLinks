// Links Web - Simulation Logic

// Helper for C++ style array initialization
function createArray(size, fillValueFactory) {
    return new Array(size).fill(null).map(() => fillValueFactory());
}

// Enums from hecken.h (for state management)
const HeckenMode = {
    MHECKEN: 0,
    MSLIDER: 1,
    MDUALHECKEN: 2,
    MDUALSLIDER: 3,
    MDUALCRANK: 4
};

class Hecken {
    constructor() {
        this.init();

        // These would be initialized after loading a file, etc.
        // For now, just placeholder logic.
        this.ground = new Point(0,0);
        this.thetaplus = 180;
        this.next = true;
        this.rebirth = true;
        this.revcalc = true;

        this.get(0);
        // this.sander(this.optimizraw); // Complex function, placeholder
        this.trace();
        this.sim();
    }

    init() {
        // Link parameters
        this.crank = new Point(0, 0);
        this.lever = new Point(-50, -50);
        this.shift = new Point(0, 50);
        this.subcrank = new Point();
        this.sublever = new Point(0, 20);
        this.subshift = new Point();

        this.optimizraw = createArray(1000, () => new Point());
        this.bgraw = createArray(1000, () => new Point());
        this.rearleg_orbit = createArray(1000, () => new Point());
        this.frontleg_orbit = createArray(1000, () => new Point());
        this.secret_orbit = createArray(1000, () => new Point());
        this.conrodraw = createArray(1000, () => new Point());
        this.leverraw = createArray(1000, () => new Point());
        this.crankraw = createArray(1000, () => new Point());
        this.subraw = createArray(1000, () => new Point());

        this.conroden = createArray(100, () => new Cyclo());
        this.leveren = createArray(100, () => new Cyclo());
        this.cranken = createArray(100, () => new Cyclo());
        this.suben = createArray(100, () => new Cyclo());
        this.bgen = createArray(100, () => new Cyclo());

        this.crankr = 25;
        this.leverr = 50;
        this.subcrankr = 0;
        this.subleverr = 0;
        this.conrod = 50;
        this.subconrod = 0;
        this.theta = 0;
        this.mini = 42;
        this.max = 42;
        this.step = 5;
        this.height = 100;
        this.object = 3;
        this.resol = 100;
        this.roll = 15;
        this.tilt = 0;
        this.phase = 0;
        this.thetaplus = 0;

        this.next = false;
        this.side = false;
        this.rebirth = true;
        this.turn = true;
        this.dxf = true;
        this.revcalc = false;
        this.mode = HeckenMode.MSLIDER;
        this.doublecrank = false;
        this.arcmode = false;

        // Reckoned parameters
        this.crankjoint = new Point();
        this.leverjoint = new Point();
        this.toe = new Point();
        this.subcrankjoint = new Point();
        this.subleverjoint = new Point();
        this.subtoe = new Point();
        this.ground = new Point();

        this.orbit = createArray(1000, () => new Point());
        this.optimized = createArray(1000, () => new Point());

        this.updown = 0;
        this.scale = 3;
        this.tiltold = 0;

        this.lock = false;
        this.dxfcrank = false;
        this.dxflever = false;
        this.dxfconrod = false;
        this.dxfsub = false;
        this.dxfbg = false;
        this.helpmode = false; // Simplified from cfg.first

        this.exist_frontleg_orbit = false;
        this.exist_rearleg_orbit = false;
        this.exist_secret_orbit = false;
        this.exist_maxmin_length = false;
        this.exist_graph = false;

        // UI state holders
        // In the C++ version, these are arrays of UI objects. Here, we'll
        // just store the boolean state of the menu buttons. The UI objects
        // themselves will be managed in ui.js.
        this.menu = {
            mturn: { value: true },
            mstop: { value: false },
            // ... other menu items to be added
        };
    }

    rad(deg) {
        return deg * 2 * Math.PI / 360;
    }

    // Main simulation step
    proceed(g) {
        g.clear();

        // Let UI state control simulation properties
        this.turn = this.menu.mturn.value;

        if (this.menu.mstop.value) { // 'next' in C++
            // The '60' is a framerate assumption (60fps), roll is in RPM.
            // This makes the speed roughly correct if the animation runs at 60fps.
            const speedFactor = (this.roll / 60) * (360 / 60);
            this.theta = (this.theta + (this.turn ? 1 : -1) * speedFactor) % 360;
        }

        this.drawOnce(g, true); // Draw background elements

        for (let i = 0; i < this.object; i++) {
            this.get(this.theta + i * 360 / this.object);
            // Color calculation is complex, use a placeholder
            const color = `hsl(${(i * 360 / this.object)}, 100%, 50%)`;
            this.draw(g, color, '#555');
        }

        this.drawOnce(g, false); // Draw foreground elements
    }

    // Kinematic calculation
    get(deg) {
        const angleRad = this.rad(deg);
        this.crankjoint.x = this.crank.x + this.crankr * Math.cos(angleRad);
        this.crankjoint.y = this.crank.y + this.crankr * Math.sin(angleRad);

        const dix = this.crankjoint.x - this.lever.x;
        const diy = this.crankjoint.y - this.lever.y;
        const part = Math.hypot(dix, diy);

        if (this.leverr === 0 || this.mode === HeckenMode.MSLIDER) {
            if (part === 0) part = 1;
            this.leverjoint.x = this.crankjoint.x - this.conrod * dix / part;
            this.leverjoint.y = this.crankjoint.y - this.conrod * diy / part;
            this.lock = false;
        } else {
            // This is the 4-bar linkage (Hecken mode) calculation
            if (part === 0 || this.conrod === 0) {
                this.lock = true;
                return;
            }
            const cosI = dix / part;
            const sinI = (this.side ? 1 : -1) * diy / part;

            const cosO_arg = (part**2 + this.conrod**2 - this.leverr**2) / (2 * this.conrod * part);

            if (Math.abs(cosO_arg) > 1) {
                this.lock = true;
            } else {
                this.lock = false;
                const cosO = cosO_arg;
                const sinO = Math.sqrt(1.0 - cosO**2);

                const new_dx = (cosO * cosI - sinO * sinI) * this.conrod;
                const new_dy = (sinO * cosI + cosO * sinI) * this.conrod;

                this.leverjoint.x = this.crankjoint.x - new_dx;
                this.leverjoint.y = this.crankjoint.y - ((this.side ? 1 : -1) * new_dy);
            }
        }

        this.toe = this.crankjoint.offset(this.leverjoint, this.shift);

        // After calculating the main points, do the 'grounding' step
        // which processes the raw optimization data into the final 'optimized' path
        if (!this.dxf) {
            if (this.mode === HeckenMode.MDUALSLIDER) {
                // Not implemented
            } else {
                this.optmax = this.grounding(this.crankjoint, this.leverjoint, this.optimizraw, this.optimized, groundlock);
            }
        }
    }

    // Drawing the mechanism
    draw(g, color, gray) {
        g.line(this.crank, this.crankjoint, gray);
        if (this.lock) return;

        // Draw the linkage itself
        g.line(this.leverjoint, this.crankjoint, color);
        g.line(this.crankjoint, this.toe, color);
        if (this.leverr !== 0) g.line(this.lever, this.leverjoint, gray);

        // Draw the optimized leg shape
        if (!this.dxf) {
            g.pline(this.optimized, color);
        }
    }

    // Drawing guide lines, etc.
    drawOnce(g, first) {
        const gray = '#555';
        if (first) {
            g.circle(this.crank, this.crankr, gray, false);
            if (this.leverr !== 0) g.circle(this.lever, this.leverr, gray, false);
            if (this.shift.x !== 0 || this.shift.y !== 0) {
                 // In the original, this draws the 'orbit' path.
                 // We will implement trace() later to calculate this path.
                 g.pline(this.orbit, gray);
            }
        } else {
            // Draw UI elements, help text etc.
            // This will be handled by ui.js
        }
    }

    // Calculate the path of the 'toe'
    trace() {
        for (let i = 0; i <= this.resol; i++) {
            this.get(this.theta + i * 360 / this.resol, true);
            this.orbit[i] = this.toe.clone();
            this.orbit[i].heel = !this.lock;
        }
        this.orbit[this.resol + 1] = this.orbit[this.resol];
    }

    // Placeholder for simulation analysis
    sim() {
        // TODO: Implement logic from hecken::sim
    }

    // From hecken::grounding
    grounding(datam, sub, raw, ed, groundlock) {
        const dist = datam.dist(sub);
        let i = 0;
        for (i = 0; i < raw.length -1; i++) {
            ed[i] = datam.offset(sub, raw[i]);
            ed[i].heel = raw[i].heel;

            if (this.ground.y < ed[i].y && !this.lock && !groundlock) {
                this.ground = ed[i].clone();
            }
            if (raw[i].equals(raw[i+1])) {
                ed[i+1] = ed[i].clone();
                break;
            }
        }
        return i;
    }

    runOptimization() {
        console.log("Running optimization...");
        this.sander(this.optimizraw);
        this.dxf = false; // Switch to optimized view
        this.rebirth = true;
        console.log("Optimization complete.");
    }

    switchSide() {
        this.side = !this.side;
        this.rebirth = true;
    }

    // From hecken::sander
    sander(raw) {
        const POINT_LIM = 1000;
        let len = new Array(POINT_LIM).fill(10000);
        raw.forEach(p => { p.x = p.y = 10000; });

        const dualmode = (this.mode === HeckenMode.MDUALSLIDER || this.mode === HeckenMode.MDUALHECKEN);

        for (let i = 0; i <= this.resol; i++) {
            this.get(this.theta + i * 360 / this.resol, true);

            const temp_leverjoint = dualmode ? this.subleverjoint : this.leverjoint;
            const temp_crankjoint = dualmode ? this.toe : this.crankjoint;
            const temp_conrod = dualmode ? this.subconrod : this.conrod;

            const slope = temp_leverjoint.__dir(temp_crankjoint);

            let leg_cnt = 0;
            const mini_rad = this.rad(this.mini);
            const max_rad = this.rad(this.max);
            const step_rad = this.rad(this.step);

            for (let line_ang = slope + Math.PI * 0.5 + mini_rad; line_ang <= slope + Math.PI * 1.5 - max_rad; line_ang += step_rad) {
                if (leg_cnt >= POINT_LIM) break;

                const sin_line_ang = Math.sin(line_ang);
                if (Math.abs(sin_line_ang) < 1e-9) continue;

                let length = (this.crank.y + this.height - temp_leverjoint.y) / sin_line_ang;

                if (length < len[leg_cnt] && length > 0) {
                    len[leg_cnt] = length;
                    const result_point = sincos(line_ang - slope - Math.PI / 2).multiply(length);
                    raw[leg_cnt] = result_point.subtract(new Point(0, temp_conrod));
                }
                raw[leg_cnt].heel = true;
                leg_cnt++;
            }
            if (leg_cnt > 0 && leg_cnt < POINT_LIM) {
                 raw[leg_cnt] = raw[leg_cnt - 1].clone();
            }
        }
    }

    // Load parameters from a .links file content
    open(fileContent) {
        const lines = fileContent.split('\n');
        const params = lines.map(line => parseFloat(line.split('\t')[0]));

        let i = 0;
        this.resol = params[i++] || 100;
        this.theta = params[i++] || 0;
        this.object = params[i++] || 3;
        this.scale = params[i++] || 3;
        this.tilt = params[i++] || 0;
        this.roll = params[i++] || 15;

        this.dxf = Boolean(params[i++]);
        this.side = Boolean(params[i++]);

        this.crankr = params[i++] || 25;
        this.conrod = params[i++] || 50;
        this.leverr = params[i++] || 50;
        this.lever.x = params[i++] || -50;
        this.lever.y = params[i++] || -50;
        this.shift.x = params[i++] || 0;
        this.shift.y = params[i++] || 50;

        this.height = params[i++] || 100;
        this.mini = params[i++] || 42;
        this.max = params[i++] || 42;
        this.step = params[i++] || 5;

        // ... and so on for all parameters

        console.log("Loaded parameters from file.");
        this.rebirth = true; // Trigger a recalculation
        this.trace();
        this.sim();
    }

    // Save parameters to a string in .links format
    save() {
        const lines = [
            `${this.resol}\tResolution`,
            `${this.theta}\tAngle`,
            `${this.object}\tObject Count`,
            `${this.scale}\tScale`,
            `${this.tilt}\tTilt`,
            `${this.roll}\tRPM`,
            `${Number(this.dxf)}\tDXF Mode`,
            `${Number(this.side)}\tSide`,
            `${this.crankr}\tCrank Radius`,
            `${this.conrod}\tConrod Length`,
            `${this.leverr}\tLever Length`,
            `${this.lever.x}\tLever X`,
            `${this.lever.y}\tLever Y`,
            `${this.shift.x}\tShift X`,
            `${this.shift.y}\tShift Y`,
            `${this.height}\tHeight`,
            `${this.mini}\tMini`,
            `${this.max}\tMax`,
            `${this.step}\tStep`,
            // ... and so on for all parameters
        ];
        return lines.join('\n') + '\neof';
    }
}
