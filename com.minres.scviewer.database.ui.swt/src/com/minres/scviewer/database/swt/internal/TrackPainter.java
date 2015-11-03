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

import java.util.Map.Entry;
import java.util.TreeMap;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Rectangle;

public class TrackPainter implements IPainter {
	
	/**
	 * 
	 */
	private final WaveformCanvas waveCanvas;

	/**
	 * @param txDisplay
	 */
	public TrackPainter(WaveformCanvas waveCanvas) {
		this.waveCanvas = waveCanvas;
	}

	public void paintArea(GC gc, Rectangle a) {
	    Rectangle area = new Rectangle(a.x, a.y+waveCanvas.rulerHeight, a.width, a.height-waveCanvas.rulerHeight);
		gc.setBackground(this.waveCanvas.colors[WaveformCanvas.Colors.TRACK_BG_EVEN.ordinal()]);
		gc.setFillRule(SWT.FILL_EVEN_ODD);
		gc.fillRectangle(area);
		TreeMap<Integer, IWaveformPainter> offsets = this.waveCanvas.trackVerticalOffset;
		if(this.waveCanvas.streams.size()>0){
			Integer firstKey=offsets.floorKey(area.y);
			if(firstKey==null) firstKey=offsets.firstKey();
			Integer lastKey = offsets.floorKey(area.y+area.height);
			Rectangle subArea = new Rectangle(area.x, 0, area.width, 0);
			if(lastKey==firstKey){
				subArea.y=firstKey;
				IWaveformPainter p = offsets.get(firstKey);
				subArea.height=p.getMinHeight();
				p.paintArea(gc, subArea);
			}else{
				for(Entry<Integer, IWaveformPainter> entry : offsets.subMap(firstKey, true, lastKey, true).entrySet()){
					subArea.y=entry.getKey();
					subArea.height=entry.getValue().getMinHeight();
					entry.getValue().paintArea(gc, subArea);
				}
			}
		}
	}
}