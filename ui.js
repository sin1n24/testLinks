// Links Web - UI Management

class UI {
    constructor(simulation, graphics) {
        this.sim = simulation;
        this.graphics = graphics;
        this.paramPanel = document.getElementById('param-panel');
        this.menuBar = document.getElementById('menu-bar');
        this.fileIoContainer = document.getElementById('file-io');
        this.helpText = document.getElementById('help-text-container');
        this.controls = {}; // To store references to the input elements

        this.defineControls();
        this.createControls();
    }

    setupFileHandlers(loadFileCallback, saveFileCallback) {
        const fileInput = document.getElementById('file-input');
        fileInput.addEventListener('change', (event) => {
            const file = event.target.files[0];
            if (!file) return;

            const reader = new FileReader();
            reader.onload = (e) => {
                loadFileCallback(e.target.result);
            };
            reader.readAsText(file);
        });

        const saveButton = document.createElement('button');
        saveButton.innerText = 'Save .links';
        saveButton.addEventListener('click', saveFileCallback);
        this.fileIoContainer.appendChild(saveButton);
    }

    defineControls() {
        // Based on the enums and initInterface in hecken.h
        this.menuButtons = [
            { id: 'mstop', text: 'Stop/Start', toggle: true },
            { id: 'mturn', text: 'Reverse', toggle: true },
            { id: 'mside', text: 'Switch Side', toggle: false }, // push button
            // Add more buttons as needed
        ];

        this.paramSliders = [
            { id: 'crankr', label: 'Crank Radius', obj: this.sim, prop: 'crankr', min: 0, max: 100, step: 1 },
            { id: 'conrod', label: 'Conrod Length', obj: this.sim, prop: 'conrod', min: 0, max: 150, step: 1 },
            { id: 'leverr', label: 'Lever Length', obj: this.sim, prop: 'leverr', min: 0, max: 150, step: 1 },
            { id: 'shiftx', label: 'Shift X', obj: this.sim.shift, prop: 'x', min: -100, max: 100, step: 1 },
            { id: 'shifty', label: 'Shift Y', obj: this.sim.shift, prop: 'y', min: -100, max: 100, step: 1 },
            { id: 'resol', label: 'Resolution', obj: this.sim, prop: 'resol', min: 20, max: 500, step: 10 },
            { id: 'roll', label: 'RPM', obj: this.sim, prop: 'roll', min: 1, max: 60, step: 1 },
            { id: 'object', label: 'Object Count', obj: this.sim, prop: 'object', min: 1, max: 10, step: 1 },
            { id: 'scale', label: 'Zoom', obj: this.graphics, prop: 'scale', min: 0.5, max: 10, step: 0.1 },
        ];
    }

    createControls() {
        // Create Menu Buttons
        this.menuButtons.forEach(b => {
            const button = document.createElement('button');
            button.innerText = b.text;
            this.sim.menu[b.id] = { value: b.toggle ? this.sim.menu[b.id]?.value ?? false : false };

            if (b.toggle) {
                // Toggle style based on state
                if (this.sim.menu[b.id].value) button.classList.add('active');
            }

            button.addEventListener('click', () => {
                if (b.toggle) {
                    this.sim.menu[b.id].value = !this.sim.menu[b.id].value;
                    button.classList.toggle('active');
                } else {
                    // For non-toggle buttons, just call a method
                    if (this.sim[b.id]) this.sim[b.id]();
                }
            });
            this.menuBar.appendChild(button);
        });

        // Create Parameter Sliders
        this.paramSliders.forEach(p => {
            const container = document.createElement('div');
            container.className = 'param-item';

            const label = document.createElement('label');
            label.innerText = p.label;

            const slider = document.createElement('input');
            slider.type = 'range';
            slider.min = p.min;
            slider.max = p.max;
            slider.step = p.step;
            slider.value = p.obj[p.prop];

            const valueSpan = document.createElement('span');
            valueSpan.innerText = slider.value;

            slider.addEventListener('input', () => {
                const val = parseFloat(slider.value);
                p.obj[p.prop] = val;
                valueSpan.innerText = val.toFixed(p.step < 1 ? 2 : 0);
            });

            container.appendChild(label);
            container.appendChild(slider);
            container.appendChild(valueSpan);
            this.paramPanel.appendChild(container);

            this.controls[p.id] = { slider, valueSpan };
        });
    }

    update() {
        // Update sliders if values are changed elsewhere
        this.paramSliders.forEach(p => {
            const control = this.controls[p.id];
            if (control) {
                const simValue = p.obj[p.prop];
                if (parseFloat(control.slider.value) !== simValue) {
                    control.slider.value = simValue;
                    control.valueSpan.innerText = simValue.toFixed(p.step < 1 ? 2 : 0);
                }
            }
        });
    }
}
