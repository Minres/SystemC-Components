/*******************************************************************************
 * Copyright (c) 2012 IT Just working.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IT Just working - initial API and implementation
 *******************************************************************************/
package com.itjw.txviewer.graph.ui.swt;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.TreeMap;
import java.util.Vector;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.TypedListener;

import com.itjw.txviewer.database.ITransaction;
import com.itjw.txviewer.graph.TxEditorPlugin;
import com.itjw.txviewer.graph.data.ITrStreamFacade;
import com.itjw.txviewer.graph.data.ITransactionFacade;

public class WaveImageCanvas extends SWTImageCanvas{

	static final int rulerHeight = 20;
	static final int rulerTickMinor = 10;
	static final int rulerTickMajor = 100;
	static final int trackHeight = 50;
	static final int trackInset =2;
	static final int txHeight = trackHeight-2*trackInset;

	private TxEditorPlugin plugin;
	private Rectangle size = new Rectangle(0,0,0,0);
	private TreeMap<Integer, ITrStreamFacade> loc2str;
	private TreeMap<Long, Rectangle> strId2bb;
 
    private ITransactionFacade currentSelection;  

	public WaveImageCanvas(final Composite parent) {
		this(parent, SWT.NONE);
	}

	public WaveImageCanvas(final Composite parent, int style) {
		super(parent, style | SWT.V_SCROLL | SWT.H_SCROLL | SWT.NO_BACKGROUND);
		plugin=TxEditorPlugin.getDefault();	
		loc2str=new TreeMap<Integer, ITrStreamFacade>();
		strId2bb=new TreeMap<Long, Rectangle>();
		center=false;
	}
	
	public void setWaveformList(LinkedList<ITrStreamFacade> linkedList){
		double maxTime=1.0;
		int height=rulerHeight;
		for(ITrStreamFacade str:linkedList){
			maxTime = Math.max(maxTime, str.getDb().getMaxTime().getValueNS());
			int strHeight=str.getMaxConcurrrentTx();
			str.setHeight(strHeight*trackHeight);
			height+=str.getHeight();
		}
		size = new Rectangle(0,0,(int) Math.ceil(maxTime),height);
        Image buffer = new Image(getDisplay(), size);
        GC gc = new GC(buffer);
        gc.setAntialias(SWT.ON);
        int trackOffset=rulerHeight;
        int trackIdx=0;
        Vector<Integer> rowendtime = new Vector<Integer>();
        loc2str.clear();
        strId2bb.clear();
		for(ITrStreamFacade str:linkedList){
			loc2str.put(trackOffset, str);
			rowendtime.clear();
	        rowendtime.add(0);
	        drawTrack(gc, new Rectangle(0, trackOffset, size.width, trackHeight), trackIdx);
			for(ITransaction tx: str.getTransactions()){
		        int rowIdx=0;
				Integer beginTime = (int)(tx.getBeginTime().getValueNS());
				Integer endTime = (int)(tx.getEndTime().getValueNS());
				for(rowIdx=0; rowendtime.size()<rowIdx || rowendtime.get(rowIdx)>beginTime; rowIdx++);
				if(rowendtime.size()<=rowIdx){
					rowendtime.add(endTime!=null?endTime:beginTime+1);
			        drawTrack(gc, new Rectangle(0, trackOffset+rowIdx*trackHeight, size.width, trackHeight), trackIdx);
				} else {
					rowendtime.set(rowIdx, endTime!=null?endTime:beginTime+1);
				}
				int width = endTime!=null?endTime-beginTime:1;
				Rectangle bb = new Rectangle(beginTime, trackOffset+rowIdx*trackHeight+trackInset, width, txHeight);
				strId2bb.put(tx.getId(), bb);
				drawTx(gc, bb);
			}
			trackOffset+=rowendtime.size()*trackHeight;
			trackIdx++;
		}
        gc.dispose();
        setSourceImage(buffer);
        redraw();
        syncScrollBars();
	}
	
	private void drawTx(GC gc, Rectangle bb){
        gc.setForeground(plugin.getColor(TxEditorPlugin.lineColor));
        gc.setFillRule(SWT.FILL_EVEN_ODD);
        gc.setBackground(plugin.getColor(TxEditorPlugin.txBgColor));
        gc.setLineWidth(1);
        gc.setLineStyle(SWT.LINE_SOLID);
		if(bb.width<8){
			gc.fillRectangle(bb);
			gc.drawRectangle(bb);
		} else {
	        gc.fillRoundRectangle(bb.x, bb.y, bb.width, bb.height, 4, 4);
	        gc.drawRoundRectangle(bb.x, bb.y, bb.width, bb.height, 4, 4);
		}	
	}
	
	private void drawHighliteTx(GC gc, Rectangle bb){
        gc.setForeground(plugin.getColor(TxEditorPlugin.highliteLineColor));
        gc.setFillRule(SWT.FILL_EVEN_ODD);
        gc.setBackground(plugin.getColor(TxEditorPlugin.txHighliteBgColor));
        gc.setLineWidth(1);
        gc.setLineStyle(SWT.LINE_SOLID);
		if(bb.width<10){
			gc.fillRectangle(bb);
			gc.drawRectangle(bb);
		} else {
	        gc.fillRoundRectangle(bb.x, bb.y, bb.width, bb.height, 5, 5);
	        gc.drawRoundRectangle(bb.x, bb.y, bb.width, bb.height, 5, 5);
		}	
	}
	
	private void drawTrack(GC gc, Rectangle bb, int trackIdx){
        gc.setForeground(plugin.getColor(TxEditorPlugin.lineColor));
        gc.setFillRule(SWT.FILL_EVEN_ODD);
        gc.setBackground(trackIdx%2==0?
        		plugin.getColor(TxEditorPlugin.trackBgLightColor):
        			plugin.getColor(TxEditorPlugin.trackBgDarkColor));
        gc.setLineWidth(1);
        gc.setLineStyle(SWT.LINE_SOLID);
		gc.fillRectangle(bb);
		gc.drawLine(bb.x, bb.y+bb.height/2, bb.width, bb.y+bb.height/2);
	}

	protected void postImagePaint(GC gc){
		Rectangle imageRect = inverseTransformRect(getTransform(), getClientArea());
		if(currentSelection!=null){
			Rectangle bb = strId2bb.get(currentSelection.getId());
			if(bb != null) {
				drawHighliteTx(gc, new Rectangle(bb.x-imageRect.x, bb.y-imageRect.y, bb.width, bb.height));
			} else
				System.err.print("No bounding box for transaction "+currentSelection.getId()+" found!");
		}
		drawRuler(gc, imageRect);
	}

	private void drawRuler(GC gc, Rectangle clientRect) {
		int startMinorIncr = ((int)(clientRect.x/rulerTickMinor))*rulerTickMinor;
		gc.setBackground(getDisplay().getSystemColor (SWT.COLOR_WIDGET_BACKGROUND));
		gc.fillRectangle(new Rectangle(-clientRect.x, 0, clientRect.width, rulerHeight));
		gc.setBackground(plugin.getColor(TxEditorPlugin.headerBgColor));
		gc.fillRectangle(new Rectangle(-clientRect.x, 0, clientRect.width, rulerHeight-1));
		gc.setForeground(plugin.getColor(TxEditorPlugin.headerFgColor));
		gc.drawLine(-clientRect.x, rulerHeight-2, clientRect.width+clientRect.x, rulerHeight-2);
		for(int x=startMinorIncr; x<(clientRect.x+clientRect.width); x+=rulerTickMinor){
			if((x%rulerTickMajor)==0){
				gc.drawLine(x-clientRect.x, 10, x-clientRect.x, rulerHeight-2);
				gc.drawText(Integer.toString(x), x-clientRect.x, 0);
			}else{
				gc.drawLine(x-clientRect.x, 15, x-clientRect.x, rulerHeight-2);
			}
		}
	}

	private ITransaction getTrAtTime(ITrStreamFacade str, double t){
		ITransaction res=null;
		Iterator<ITransaction> iter = str.getTransactions().iterator();
		while(iter.hasNext()){
			res = iter.next();
			if(res.getEndTime().getValueNS()>=t) break;
		}
		if(res!=null && res.getBeginTime().getValueNS()-10<t)
			return res;
		return null;
	}
	
	public void addSelectionListener(SelectionListener listener) {
		checkWidget();
		if (listener != null) {
			TypedListener typedListener = new TypedListener(listener);
			addListener(SWT.Selection,typedListener);
			addListener(SWT.DefaultSelection,typedListener);
		}
	}

	public ITransactionFacade getTransactionAtPos(Point ps){
		Point p=SWTImageCanvas.inverseTransformPoint(getTransform(), ps);
		ITrStreamFacade str = loc2str.floorEntry(p.y).getValue();
		ITransaction tr = getTrAtTime(str, p.x);
//		System.out.print("Clicked on "+str.getFullName()+" at time "+p.x+"ns");
		if(tr != null) {
//			System.out.println(" and found Tx "+tr.getId());
			return (tr instanceof ITransactionFacade)?(ITransactionFacade)tr:new ITransactionFacade(tr);
		} else {
//			System.out.println("");
			return null;
		}
	}

	public ITransactionFacade getCurrentSelection() {
		return currentSelection;
	}

	public void setCurrentSelection(ITransactionFacade currentSelection) {
		this.currentSelection = currentSelection;
	}
	
}
