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

import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Path;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.widgets.Display;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxRelation;
import com.minres.scviewer.database.ITxStream;
import com.minres.scviewer.database.RelationType;
import com.minres.scviewer.database.ui.WaveformColors;

public class ArrowPainter implements IPainter {
	
	private final int xCtrlOffset = 50;

	private final int yCtrlOffset = 30;

	private WaveformCanvas waveCanvas;

	private ITx tx;

	private List<LinkEntry> iRect;

	private List<LinkEntry> oRect;

	private Rectangle txRectangle;

	private RelationType highlightType;

	long scaleFactor;

	boolean deferredUpdate;

	public ArrowPainter(WaveformCanvas waveCanvas, RelationType relationType) {
		this.waveCanvas = waveCanvas;
		highlightType=relationType;
		setTx(null);
	}

	public RelationType getHighlightType() {
		return highlightType;
	}

	public void setHighlightType(RelationType highlightType) {
		this.highlightType = highlightType;
	}

	public ITx getTx() {
		return tx;
	}

	public void setTx(ITx newTx) {
		this.tx = newTx;
		iRect = new LinkedList<>();
		oRect = new LinkedList<>();
		scaleFactor = waveCanvas.getScaleFactor();
		if (tx != null) {
			calculateGeometries();
		}
	}

	protected void calculateGeometries() {
		deferredUpdate = false;
		ITxStream<?> stream = tx.getStream();
		IWaveformPainter painter = waveCanvas.wave2painterMap.get(stream);
		if (painter == null) { // stream has been added but painter not yet
								// created
			deferredUpdate = true;
			return;
		}
		int laneHeight = painter.getHeight() / stream.getMaxConcurrency();
		txRectangle = new Rectangle((int) (tx.getBeginTime() / scaleFactor),
				waveCanvas.rulerHeight + painter.getVerticalOffset() + laneHeight * tx.getConcurrencyIndex(),
				(int) ((tx.getEndTime() - tx.getBeginTime()) / scaleFactor), laneHeight);
		deriveGeom(tx.getIncomingRelations(), iRect, false);
		deriveGeom(tx.getOutgoingRelations(), oRect, true);
	}

	protected void deriveGeom(Collection<ITxRelation> relations, List<LinkEntry> res, boolean useTarget) {
		for (ITxRelation iTxRelation : relations) {
			ITx otherTx = useTarget ? iTxRelation.getTarget() : iTxRelation.getSource();
			if (waveCanvas.wave2painterMap.containsKey(otherTx.getStream())) {
				ITxStream<?> stream = otherTx.getStream();
				IWaveformPainter painter = waveCanvas.wave2painterMap.get(stream);
				int laneHeight = painter.getHeight() / stream.getMaxConcurrency();
				Rectangle bb = new Rectangle((int) (otherTx.getBeginTime() / scaleFactor),
						waveCanvas.rulerHeight + painter.getVerticalOffset()
								+ laneHeight * otherTx.getConcurrencyIndex(),
						(int) ((otherTx.getEndTime() - otherTx.getBeginTime()) / scaleFactor), laneHeight);
				res.add(new LinkEntry(bb, iTxRelation.getRelationType()));
			}
		}
	}

	@Override
	public void paintArea(GC gc, Rectangle area) {
		Color fgColor = waveCanvas.colors[WaveformColors.REL_ARROW.ordinal()];
		Color highliteColor = waveCanvas.colors[WaveformColors.REL_ARROW_HIGHLITE.ordinal()];

		if (deferredUpdate || (tx != null && waveCanvas.getScaleFactor() != scaleFactor)) {
			scaleFactor = waveCanvas.getScaleFactor();
			calculateGeometries();
		}
		for (LinkEntry entry : iRect) {
			Point target = drawPath(gc, highlightType.equals(entry.relationType) ? highliteColor : fgColor,
					entry.rectangle, txRectangle);
			drawArrow(gc, target);
		}
		for (LinkEntry entry : oRect) {
			Point target = drawPath(gc, highlightType.equals(entry.relationType) ? highliteColor : fgColor, txRectangle,
					entry.rectangle);
			drawArrow(gc, target);
		}
	}

	protected void drawArrow(GC gc, Point target) {
		gc.drawLine(target.x - 8, target.y - 5, target.x, target.y);
		gc.drawLine(target.x - 8, target.y + 5, target.x, target.y);
	}

	protected Point drawPath(GC gc, Color fgColor, Rectangle srcRectangle, Rectangle tgtRectangle) {
		Point point1 = new Point(0, srcRectangle.y + srcRectangle.height / 2);
		Point point2 = new Point(0, tgtRectangle.y + tgtRectangle.height / 2);

		point1.x = srcRectangle.x;
		point2.x = tgtRectangle.x;

		if (point2.x > point1.x + srcRectangle.width)
			point1.x += srcRectangle.width;
		if (point1.x > point2.x + tgtRectangle.width)
			point2.x += tgtRectangle.width;

		Path path = new Path(Display.getCurrent());
		path.moveTo(point1.x, point1.y);
		if (point1.y == point2.y) {
			Point center = new Point((point1.x + point2.x) / 2, point1.y - yCtrlOffset);
			path.cubicTo(point1.x + xCtrlOffset, point1.y, center.x - xCtrlOffset, center.y, center.x, center.y);
			path.cubicTo(center.x + xCtrlOffset, center.y, point2.x - xCtrlOffset, point2.y, point2.x, point2.y);
		} else
			path.cubicTo(point1.x + xCtrlOffset, point1.y, point2.x - xCtrlOffset, point2.y, point2.x, point2.y);
		gc.setAntialias(SWT.ON);
		gc.setForeground(fgColor);
		gc.drawPath(path);
		path.dispose();
		return point2;
	}

	class LinkEntry {
		public Rectangle rectangle;
		public RelationType relationType;

		public LinkEntry(Rectangle rectangle, RelationType relationType) {
			super();
			this.rectangle = rectangle;
			this.relationType = relationType;
		}
	}

}
