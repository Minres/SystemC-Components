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

import com.minres.scviewer.database.ui.WaveformColors;

public class TrackAreaPainter implements IPainter {
	
	/**
	 * 
	 */
	private final WaveformCanvas waveCanvas;

	TreeMap<Integer, IWaveformPainter> trackVerticalOffset;
	/**
	 * @param txDisplay
	 */
	public TrackAreaPainter(WaveformCanvas waveCanvas) {
		this.waveCanvas = waveCanvas;
		this.trackVerticalOffset= new TreeMap<>();
	}

	public void paintArea(GC gc, Rectangle a) {
	    Rectangle area = new Rectangle(a.x, a.y+waveCanvas.rulerHeight, a.width, a.height-waveCanvas.rulerHeight);
		gc.setBackground(this.waveCanvas.colors[WaveformColors.TRACK_BG_EVEN.ordinal()]);
		gc.setFillRule(SWT.FILL_EVEN_ODD);
		gc.fillRectangle(area);
		if(trackVerticalOffset.size()>0){
			Integer firstKey=trackVerticalOffset.floorKey(area.y);
			if(firstKey==null) firstKey=trackVerticalOffset.firstKey();
			Integer lastKey = trackVerticalOffset.floorKey(area.y+area.height);
			Rectangle subArea = new Rectangle(area.x, 0, area.width, 0);
			if(lastKey==firstKey){
				subArea.y=firstKey;
				IWaveformPainter p = trackVerticalOffset.get(firstKey);
				subArea.height=p.getHeight();
				p.paintArea(gc, subArea);
			}else{
				for(Entry<Integer, IWaveformPainter> entry : trackVerticalOffset.subMap(firstKey, true, lastKey, true).entrySet()){
					subArea.y=entry.getKey();
					subArea.height=entry.getValue().getHeight();
					entry.getValue().paintArea(gc, subArea);
				}
			}
		}
	}

	public TreeMap<Integer, IWaveformPainter> getTrackVerticalOffset() {
		return trackVerticalOffset;
	}
	
	public void addTrackPainter(IWaveformPainter trackPainter){
		trackVerticalOffset.put(trackPainter.getVerticalOffset()+waveCanvas.rulerHeight, trackPainter);
		
	}
	
	public int getHeight(){
		if(trackVerticalOffset.size()==0) return 1;
		return trackVerticalOffset.lastKey() + trackVerticalOffset.lastEntry().getValue().getHeight();
	}
}