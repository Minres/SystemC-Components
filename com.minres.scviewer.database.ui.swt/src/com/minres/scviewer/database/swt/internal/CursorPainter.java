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

import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Rectangle;

import com.minres.scviewer.database.ui.ICursor;
import com.minres.scviewer.database.ui.WaveformColors;

public class CursorPainter implements IPainter, ICursor {

	/**
	 * 
	 */
	private final WaveformCanvas waveCanvas;
	
	private long time;

	private boolean isDragging;
	
	public final int id;
	
	/**
	 * @param i 
	 * @param txDisplay
	 */
	public CursorPainter(WaveformCanvas txDisplay, long time, int id) {
		this.waveCanvas = txDisplay;
		this.time=time;
		this.id=id;
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
		if(this.waveCanvas.painterList.size()>0){
			long scaleFactor=waveCanvas.getScaleFactor();
			int x = (int) (time/scaleFactor);
			int top = id<0?area.y:area.y+15;
			Color drawColor=waveCanvas.colors[id<0?WaveformColors.CURSOR.ordinal():WaveformColors.MARKER.ordinal()];
			Color dragColor = waveCanvas.colors[WaveformColors.CURSOR_DRAG.ordinal()];
			Color textColor=waveCanvas.colors[id<0?WaveformColors.CURSOR_TEXT.ordinal():WaveformColors.MARKER_TEXT.ordinal()];
			if(x>=area.x && x<=(area.x+area.width)){
				gc.setForeground(isDragging?dragColor:drawColor);
				gc.drawLine(x, top, x, area.y+area.height);
				gc.setBackground(drawColor);
				gc.setForeground(textColor);
				Double dTime=new Double(time);
				gc.drawText((dTime/waveCanvas.getScaleFactorPow10())+waveCanvas.getUnitStr(), x+1, top);
			}
		}
	}

}