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
