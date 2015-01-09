package com.minres.scviewer.ui.swt;

import java.util.HashMap;
import java.util.NavigableSet;
import java.util.Vector;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Layout;

import com.minres.scviewer.database.EventTime;
import com.minres.scviewer.database.ITx;
import com.minres.scviewer.ui.TxEditorPlugin;

public class Track extends Composite implements IWaveformWidget, MouseListener {

	static final int trackHeight = 50;
	static final int trackInset = 1;
	static final int txHeight = trackHeight - 2 * trackInset;

	static double zoomFactor = EventTime.getScalingFactor(EventTime.Unit.NS);
	private Color lineColor;
	private Color trackBgColor;
	
	private ITx highlightedTx=null;
	
	private HashMap<ITx, Transaction> transactionMap =  new HashMap<ITx, Transaction>();
	
	class TrackLayoutData {
		protected int x, y;
		TrackLayoutData(int x, int y){
			this.x=x;
			this.y=y;
		}
		public int getX() {
			return x;
		}
		public int getY() {
			return y;
		}
		
	}
	
	class TrackLayout extends Layout {
		Point extent; // the cached sizes

		protected Point computeSize(Composite composite, int wHint, int hHint, boolean changed) {
			if (changed || extent == null) {
				extent=new Point(0, 0);
				for(Control child:composite.getChildren()){
					Point cExtent = child.computeSize(SWT.DEFAULT, SWT.DEFAULT, false);
					TrackLayoutData dat = (TrackLayoutData) child.getLayoutData();
					extent.x=Math.max(extent.x, dat.x+cExtent.x);
					extent.y=Math.max(extent.y, dat.y+cExtent.y);
				}
			}
			return extent;
		}

		protected void layout(Composite composite, boolean changed) {
			if(extent==null){
				extent=new Point(0, 0);
				changed=true;
			}
			for(Control child:composite.getChildren()){
				Point cExtent = child.computeSize(SWT.DEFAULT, SWT.DEFAULT, false);
				TrackLayoutData dat = (TrackLayoutData) child.getLayoutData();
				if(changed){
					extent.x=Math.max(extent.x, dat.x+cExtent.x);
					extent.y=Math.max(extent.y, dat.y+cExtent.y);
				}
				child.setBounds(dat.x, dat.y, cExtent.x, cExtent.y);
			}
		}
	}

	Track(Composite parent, int style) {
		super(parent, style);
		setLayout(new TrackLayout());
		addDisposeListener(new DisposeListener() {
			public void widgetDisposed(DisposeEvent e) {
				Track.this.widgetDisposed(e);
			}
		});
		addPaintListener(new PaintListener() {
			public void paintControl(PaintEvent e) {
				Track.this.paintControl(e);
			}
		});
		TxEditorPlugin plugin=TxEditorPlugin.getDefault();	
		lineColor=plugin.getColor(TxEditorPlugin.lineColor);
		trackBgColor=plugin.getColor(TxEditorPlugin.trackBgDarkColor);
	}

	
	protected void widgetDisposed(DisposeEvent e) {
	}
	
	void paintControl(PaintEvent e) {
		GC gc = e.gc;
		gc.setForeground(lineColor);
		gc.setFillRule(SWT.FILL_EVEN_ODD);
		gc.setBackground(trackBgColor);
		gc.setLineWidth(3);
		gc.setLineStyle(SWT.LINE_SOLID);
		gc.fillRectangle(new Rectangle(e.x, e.y, e.width, e.height));
		for(int offset=trackHeight/2; offset<e.height; offset+=trackHeight)
			gc.drawLine(0, e.y+offset, e.x+e.width, e.y+offset);
	}


	public void setTransactions(NavigableSet<ITx> transactions) {
		Vector<ITx> rowendtime = new Vector<ITx>();
		for (ITx tx : transactions) {
			int rowIdx = 0;
			for (ITx lastTx : rowendtime) {
				if((lastTx.getEndTime().getValue()-lastTx.getBeginTime().getValue())>0){
					if (lastTx.getEndTime().compareTo(tx.getBeginTime())<=0 )
						break;
				} else { 
					if (lastTx.getEndTime().compareTo(tx.getBeginTime())<0 )
						break;
				}
				rowIdx++;
			}
			if (rowendtime.size() <= rowIdx) {
				rowendtime.add(tx);
			} else {
				rowendtime.set(rowIdx, tx);
			}
			int width = (int) ((tx.getEndTime().getValue()-tx.getBeginTime().getValue())/zoomFactor);
			if(width==0) width=1;
			Transaction t = new Transaction(this, SWT.NONE, width);
			t.setLayoutData(new Track.TrackLayoutData((int) (tx.getBeginTime().getValue()/zoomFactor), rowIdx*trackHeight));
			t.setData(tx);
			t.addMouseListener(this);
			transactionMap.put(tx,  t);
		}
		layout(true,true);
	}


	@Override
	public void mouseDoubleClick(MouseEvent e) {
		Event event = new Event();
		event.type = SWT.MouseDoubleClick;
		event.display=e.display;
		event.data = e.widget;
		event.button=e.button;
		event.time=e.time;
		this.notifyListeners(SWT.MouseDoubleClick, event);
	}


	@Override
	public void mouseDown(MouseEvent e) {
		Event event = new Event();
		event.type = SWT.MouseDown;
		event.display=e.display;
		event.data = e.widget;
		event.button=e.button;
		event.time=e.time;
		this.notifyListeners(SWT.MouseDown, event);
	}


	@Override
	public void mouseUp(MouseEvent e) {
		Event event = new Event();
		event.type = SWT.MouseUp;
		event.display=e.display;
		event.data = e.widget;
		event.button=e.button;
		event.time=e.time;
		this.notifyListeners(SWT.MouseUp, event);
	}
	
	public Transaction highlight(Object obj){
		if(obj==null || obj instanceof ITx){
			ITx tx = (ITx) obj;
			if(highlightedTx!=null){
				transactionMap.get(highlightedTx).highlight(false);
				highlightedTx=null;
			}
			if(tx!=null && transactionMap.containsKey(tx)){
				Transaction trans = transactionMap.get(tx);
				trans.highlight(true);
				highlightedTx=tx;
				return trans;
			}
		}
		return null;
	}
}
