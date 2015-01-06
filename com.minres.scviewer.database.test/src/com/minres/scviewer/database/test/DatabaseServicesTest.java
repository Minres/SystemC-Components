package com.minres.scviewer.database.test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.io.File;
import java.net.URISyntaxException;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.minres.scviewer.database.IWaveformDb;
import com.minres.scviewer.database.IWaveformDbFactory;

public class DatabaseServicesTest {

	private static CountDownLatch dependencyLatch = new CountDownLatch(3);// 1 = number of dependencies required
	
	private static List<IWaveformDbFactory> services=new LinkedList<IWaveformDbFactory>();
	
	public void bind(IWaveformDbFactory factory){
		services.add(factory);
		dependencyLatch.countDown();
		// System.out.println("service added");
	}
	
	public void unbind(IWaveformDbFactory factory){
		services.remove(factory);
	}
	
	@Before
	public void setUp() throws Exception {
		 // Wait for OSGi dependencies
	    try {
	      dependencyLatch.await(10, TimeUnit.SECONDS); 
	      // Dependencies fulfilled
	    } catch (InterruptedException ex)  {
	      fail("OSGi dependencies unfulfilled");
	    }
    }

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void testVCD() throws URISyntaxException {
		File f = new File("inputs/my_db.vcd").getAbsoluteFile();
		assertTrue(f.exists());
		IWaveformDb database=null;
		for(IWaveformDbFactory factory:services){
			database = factory.createDatabase(f);
			if(database!=null) break;
		}
		assertNotNull(database);
		assertEquals(3, services.size());
		assertEquals(14,  database.getAllWaves().size());
		assertEquals(2,  database.getChildNodes().size());
	}

	@Test
	public void testTxSQLite() throws URISyntaxException {
		File f = new File("inputs/my_db.txdb").getAbsoluteFile();
		assertTrue(f.exists());
		IWaveformDb database=null;
		for(IWaveformDbFactory factory:services){
			database = factory.createDatabase(f);
			if(database!=null) break;
		}
		assertNotNull(database);
		assertEquals(3, services.size());
		assertEquals(3,  database.getAllWaves().size());
		assertEquals(3,  database.getChildNodes().size());
	}

	@Test
	public void testTxText() throws URISyntaxException {
		File f = new File("inputs/my_db.txlog").getAbsoluteFile();
		assertTrue(f.exists());
		IWaveformDb database=null;
		for(IWaveformDbFactory factory:services){
			database = factory.createDatabase(f);
			if(database!=null) break;
		}
		assertNotNull(database);
		assertEquals(3, services.size());
		assertEquals(3,  database.getAllWaves().size());
		assertEquals(3,  database.getChildNodes().size());
	}


}
