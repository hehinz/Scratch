var widths = [0.38, 0.5, 0.62];

function toggleSideWidth(direction) {
    var activeWindow = workspace.activeWindow;
    var maxArea = workspace.clientArea(KWin.MaximizeArea, activeWindow);

    // print(activeWindow.frameGeometry);

    let rect = Object.assign({}, activeWindow.frameGeometry);

    var currentRatio = rect.width / maxArea.width;
    var nextIndex = 1;

    var is_aligned = rect.height == maxArea.height;
    if (direction === "left") {
        is_aligned &= rect.x === 0;
    } else {
        is_aligned &= (Math.round(rect.x + rect.width) === maxArea.width );
    }

    if (is_aligned) {
        for (var i = 0; i < widths.length; i++) {
            if (Math.abs(currentRatio - widths[i]) < 0.03) {
                nextIndex = (i + 1) % (widths.length );
                break;
            }
        }
    }

    rect.width = maxArea.width * widths[nextIndex];
    rect.height = maxArea.height;
    rect.y = 0;
    rect.x = direction === "left" ? 0 : maxArea.width - rect.width;

    activeWindow.frameGeometry = rect;
}

registerShortcut(
    "toggleSideWidthLeft",
    "Toggle left align widths",
    "Ctrl+Alt+H",
    function() { toggleSideWidth("left"); }
);

registerShortcut(
    "toggleSideWidthRight",
    "Toggle rigt align widths",
    "Ctrl+Alt+L",
    function() { toggleSideWidth("right"); }
);
