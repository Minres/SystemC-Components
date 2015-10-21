/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.database.swt.internal;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Rectangle;

public class RulerPainter implements IPainter {
    protected WaveformCanvas waveCanvas;
    
    static final int rulerTickMinorC = 10;
    static final int rulerTickMajorC = 100;
    
    private Color headerBgColor;
    private Color headerFgColor;
   
    public RulerPainter(WaveformCanvas waveCanvas, Color headerFgColor, Color headerBgColor) {
        this.waveCanvas=waveCanvas;
        this.headerBgColor=headerBgColor;
        this.headerFgColor=headerFgColor;
    }

    @Override
    public void paintArea(GC gc, Rectangle area) {
        String unit=waveCanvas.getUnitStr();
        int unitMultiplier=waveCanvas.getUnitMultiplier();
        long scaleFactor=waveCanvas.getScaleFactor();
        long start=area.x*scaleFactor;
        long end=start+area.width*scaleFactor;

        long rulerTickMinor = rulerTickMinorC*scaleFactor;
        long rulerTickMajor = rulerTickMajorC*scaleFactor;
        int minorTickY = waveCanvas.rulerHeight-5;
        int majorTickY = waveCanvas.rulerHeight-15;
        int textY=waveCanvas.rulerHeight-20;
        int baselineY=waveCanvas.rulerHeight - 1;
        int bottom=waveCanvas.rulerHeight - 2;

        long startMinorIncr = start;
        long modulo = start % rulerTickMinor;
        startMinorIncr+=rulerTickMinor-modulo;
        
        gc.setBackground(waveCanvas.getDisplay().getSystemColor(SWT.COLOR_WIDGET_BACKGROUND));
        gc.fillRectangle(new Rectangle(area.x, area.y, area.width, waveCanvas.rulerHeight));
        gc.setBackground(headerBgColor);
        gc.fillRectangle(new Rectangle(area.x, area.y, area.width, baselineY));
        gc.setForeground(headerFgColor);
        gc.drawLine(area.x, area.y+bottom, area.x+area.width, area.y+bottom);
        
        for (long tick = startMinorIncr; tick < end; tick += rulerTickMinor) {
            int x0 = (int) (tick/scaleFactor);
            if ((tick % rulerTickMajor) == 0) {
                gc.drawText(Double.toString(tick/scaleFactor*unitMultiplier)+unit, x0, area.y+textY);
                gc.drawLine(x0, area.y+majorTickY, x0,area.y+ bottom);
            } else {
                gc.drawLine(x0, area.y+minorTickY, x0, area.y+bottom);
            }
        }
        
    }
}
