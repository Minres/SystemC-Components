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
/**
 * 
 */
package com.minres.scviewer.database.internal;

import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformDbFactory;

/**
 * @author eyck
 *
 */
public class WaveformDbFactory implements IWaveformDbFactory {

	/* (non-Javadoc)
	 * @see com.minres.scviewer.database.IWaveformDbFactory#getDatabase()
	 */
	@Override
	public IWaveformDb getDatabase() {
		return new WaveformDb();
	}

}
