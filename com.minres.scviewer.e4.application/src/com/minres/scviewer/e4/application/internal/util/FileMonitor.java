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
package com.minres.scviewer.e4.application.internal.util;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Class monitoring a {@link File} for changes.
 * 
 * @author Pascal Essiembre
 */
public class FileMonitor {

	private Timer timer;

	private Hashtable<String, FileSetMonitorTask> timerEntries;

	/**
	 * Constructor.
	 */
	public FileMonitor() {
		// Create timer, run timer thread as daemon.
		timer = new Timer(true);
		timerEntries = new Hashtable<String, FileSetMonitorTask>();
	}

	/**
	 * Adds a monitored file with a FileChangeListener.
	 * 
	 * @param listener
	 *          listener to notify when the file changed.
	 * @param fileName
	 *          name of the file to monitor.
	 * @param period
	 *          polling period in milliseconds.
	 */
	public IModificationChecker addFileChangeListener(IFileChangeListener listener, File file, long period) {
		return addFileChangeListener(listener, Arrays.asList(new File[]{file}), period);
	}

	/**
	 * Adds a monitored file with a FileChangeListener.
	 * List<File> filesToLoad
	 * @param listener
	 *          listener to notify when the file changed.
	 * @param fileName
	 *          name of the file to monitor.
	 * @param period
	 *          polling period in milliseconds.
	 */
	public IModificationChecker addFileChangeListener(IFileChangeListener listener, List<File> files, long period) {
		removeFileChangeListener(listener);
		FileSetMonitorTask task = new FileSetMonitorTask(listener, files, period);
		timerEntries.put(Integer.toHexString(listener.hashCode()), task);
		timer.schedule(task, period, period);
		return task;
	}

	/**
	 * Remove the listener from the notification list.
	 * 
	 * @param listener
	 *          the listener to be removed.
	 */
	public void removeFileChangeListener(IFileChangeListener listener) {
		FileSetMonitorTask task = timerEntries.remove(Integer.toHexString(listener.hashCode()));
		if (task != null) task.cancel();
	}

	/**
	 * Fires notification that a file changed.
	 * 
	 * @param listener
	 *          file change listener
	 * @param file
	 *          the file that changed
	 */
	protected void fireFileChangeEvent(IFileChangeListener listener, List<File> file) {
		listener.fileChanged(file);
	}

	/**
	 * File monitoring task.
	 */
	class FileSetMonitorTask extends TimerTask implements IModificationChecker{

		IFileChangeListener listener;

		private List<File> monitoredFiles;

		private List<Long> lastModifiedTimes;

		public final long period;

		public FileSetMonitorTask(IFileChangeListener listener, List<File> monitoredFiles, long period) {
			this.period=period;
			this.monitoredFiles = monitoredFiles;
			this.listener = listener;
			lastModifiedTimes= new ArrayList<>();
			for(File monitoredFile:monitoredFiles){
				Long lmt = 0L;
				try {
					lmt=monitoredFile.lastModified();
				} catch(Exception e){}
				lastModifiedTimes.add(lmt);
			}
		}

		public void run() {
			check();
		}

		public void check() {
			boolean res = false;
			for(int i=0; i<monitoredFiles.size(); ++i){
				File file = monitoredFiles.get(i);
				Long lmt = 0L;
				try {
					lmt=file.lastModified();
				} catch(Exception e){}
				if (!lmt.equals(lastModifiedTimes.get(i)))
					res |= true;
				lastModifiedTimes.set(i, lmt);
			}
			if(res)
				fireFileChangeEvent(this.listener, monitoredFiles);
		}
	}
}
