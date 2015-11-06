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

import com.minres.scviewer.database.ui.TrackEntry;

public abstract class TrackPainter implements IWaveformPainter {

	protected boolean even;

	protected TrackEntry trackEntry;

	public TrackPainter(TrackEntry trackEntry, boolean even) {
		this.trackEntry = trackEntry;
		this.even=even;
	}

	public int getHeight() {
		return trackEntry.height;
	}

	public int getVerticalOffset() {
		return trackEntry.vOffset;
	}

	public boolean isEven() {
		return even;
	}

	public TrackEntry getTrackEntry() {
		return trackEntry;
	}

}
