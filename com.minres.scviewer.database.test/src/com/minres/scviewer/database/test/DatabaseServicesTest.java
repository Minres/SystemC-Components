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
package com.minres.scviewer.database.test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.io.File;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformDbFactory;

public class DatabaseServicesTest {


	private static IWaveformDbFactory waveformDbFactory;

	private IWaveformDb waveformDb;
	
	public synchronized void setFactory(IWaveformDbFactory service) {
		waveformDbFactory = service;
	}

	public synchronized void unsetFactory(IWaveformDbFactory service) {
		if (waveformDbFactory == service) {
			waveformDbFactory = null;
		}
	}
	
	@Before
	public void setUp() throws Exception {
		waveformDb=waveformDbFactory.getDatabase();
		// Wait for OSGi dependencies
//		for (int i = 0; i < 10; i++) {
//			if (waveformDb.size() == 3) // Dependencies fulfilled
//				return;
//			Thread.sleep(1000);
//		}
//		assertEquals("OSGi dependencies unfulfilled", 3, WaveformDb.getLoaders().size());
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void testVCD() throws Exception {
		File f = new File("inputs/my_db.vcd").getAbsoluteFile();
		assertTrue(f.exists());
		waveformDb.load(f);
		assertNotNull(waveformDb);
		assertEquals(14,  waveformDb.getAllWaves().size());
		assertEquals(2,  waveformDb.getChildNodes().size());
	}

	@Test
	public void testTxSQLite() throws Exception {
		File f = new File("inputs/my_db.txdb").getAbsoluteFile();
		assertTrue(f.exists());
		waveformDb.load(f);
		assertNotNull(waveformDb);
		assertEquals(3,  waveformDb.getAllWaves().size());
		assertEquals(1,  waveformDb.getChildNodes().size());
	}

	@Test
	public void testTxText() throws Exception {
		File f = new File("inputs/my_db.txlog").getAbsoluteFile();
		assertTrue(f.exists());
		waveformDb.load(f);
		assertNotNull(waveformDb);
		assertEquals(3,  waveformDb.getAllWaves().size());
		assertEquals(1,  waveformDb.getChildNodes().size());
	}


}
