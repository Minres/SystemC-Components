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

import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Rectangle;

import com.minres.scviewer.database.ITx;
import com.minres.scviewer.database.ITxRelation;

public class ArrowPainter implements IPainter {

	private WaveformCanvas waveCanvas;
	
	private ITx tx;

	private Collection<ITxRelation> incoming;

	private Collection<ITxRelation> outgoing;

	public  ArrowPainter(WaveformCanvas waveCanvas, ITx tx) {
		this.waveCanvas = waveCanvas;
		this.tx=tx;
		this.incoming = tx.getIncomingRelations();
		this.outgoing = tx.getOutgoingRelations();

	}
	
	@Override
	public void paintArea(GC gc, Rectangle area) {
		Rectangle txRectangle = getBounds(tx);
	}

	private Rectangle getBounds(ITx tx) {
		return null;
	}

}
