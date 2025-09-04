// Links Web - Main Entry Point

window.onload = () => {
    // Get canvas and context
    const canvas = document.getElementById('main-canvas');
    if (!canvas) {
        console.error("Canvas element not found!");
        return;
    }

    // Initialize components
    const graphics = new Graphics(canvas);
    const simulation = new Hecken();

    // --- File I/O Handlers ---
    const loadFile = (content) => {
        simulation.open(content);
    };

    const saveFile = () => {
        const content = simulation.save();
        const blob = new Blob([content], { type: 'text/plain;charset=utf-8' });
        const a = document.createElement('a');
        a.href = URL.createObjectURL(blob);
        a.download = 'scene.links';
        a.click();
        URL.revokeObjectURL(a.href);
    };

    const dxfParser = new DxfParser();
    const loadDxfFile = (part, content) => {
        logMessage(`Loading DXF for part: ${part}`);
        const geometry = dxfParser.parse(content);
        simulation.loadDxf(part, geometry);
    };

    const ui = new UI(simulation, graphics, { saveFile: saveFile });
    ui.setupFileHandlers(loadFile);
    ui.setupDxfFileHandlers(loadDxfFile);

    // --- Logging ---
    const logMessage = (message) => {
        ui.addLogMessage(message);
        console.log(message); // Also keep logging to console for debugging
    };

    simulation.setLogger(logMessage);

    logMessage("Links Web application started.");
    const helpMessage = `※本ウェブアプリは開発中のβ版につき、一部機能が未実装や不安定です。</br>
                全ての機能を使いたい場合はWin版をご利用頂き、使い方についても<a href="../Links.html">Win版の取説</a>をご覧下さい。</br>
                なおWin版の開発は停止しており今後追加機能実装の予定はありません。`;
    logMessage(helpMessage);


    // Main loop
    function gameLoop(timestamp) {
        graphics.updateMouseState(); // Reset one-frame mouse flags
        simulation.proceed(graphics);
        ui.update();

        requestAnimationFrame(gameLoop);
    }

    // Start the loop
    requestAnimationFrame(gameLoop);
};
