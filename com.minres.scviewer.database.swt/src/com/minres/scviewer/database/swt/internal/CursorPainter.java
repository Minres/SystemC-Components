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

import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Rectangle;

public class CursorPainter implements IPainter {
	
	/**
	 * 
	 */
	private final WaveformCanvas waveCanvas;
	private long time;

	private boolean isDragging;
	private boolean drawTime;
	/**
	 * @param i 
	 * @param txDisplay
	 */
	public CursorPainter(WaveformCanvas txDisplay, long time) {
		this.waveCanvas = txDisplay;
		this.time=time;
		drawTime=true;
	}

	public long getTime() {
		return time;
	}

	public void setTime(long time) {
		this.time = time;
	}

	public boolean isDragging() {
        return isDragging;
    }

    public void setDragging(boolean isDragging) {
        this.isDragging = isDragging;
    }

    public void paintArea(GC gc, Rectangle area) {			
			if(this.waveCanvas.streams.size()>0){
			    long scaleFactor=waveCanvas.getScaleFactor();
				int x = (int) (time/scaleFactor);
				if(x>=area.x && x<=(area.x+area.width)){
					gc.setForeground(
					        waveCanvas.colors[isDragging?
					                WaveformCanvas.Colors.CURSOR_DRAG.ordinal():
					                    WaveformCanvas.Colors.CURSOR.ordinal()]);
					gc.drawLine(x, area.y, x, area.y+area.height);
					if(drawTime){
					    gc.setBackground(waveCanvas.colors[WaveformCanvas.Colors.CURSOR.ordinal()]);
                        gc.setForeground(waveCanvas.colors[WaveformCanvas.Colors.CURSOR_TEXT.ordinal()]);
					    gc.drawText(Double.toString(x*waveCanvas.getUnitMultiplier())+waveCanvas.getUnitStr(), x+1, area.y);
					}
				}
			}
	}
}