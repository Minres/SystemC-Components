/*******************************************************************************
 * Copyright (c) 2014, 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.ui.swt;

import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Rectangle;

class CursorPainter implements IPainter {
	
	/**
	 * 
	 */
	private final WaveformCanvas waveCanvas;
	private long time;

	/**
	 * @param i 
	 * @param txDisplay
	 */
	CursorPainter(WaveformCanvas txDisplay, long time) {
		this.waveCanvas = txDisplay;
		this.time=time;
	}

	public long getTime() {
		return time;
	}

	public void setTime(long time) {
		this.time = time;
	}

	public void paintArea(GC gc, Rectangle area) {			
			if(this.waveCanvas.streams.size()>0){
				int x = (int) (time/waveCanvas.getScaleFactor());
				if(x>=area.x && x<=(area.x+area.width)){
					gc.setForeground(waveCanvas.colors[WaveformCanvas.Colors.CURSOR.ordinal()]);
					gc.drawLine(x, area.y, x, area.y+area.height);
				}
			}
	}
}