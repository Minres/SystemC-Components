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
 */
public class FileMonitor {

	/** The timer. */
	private Timer timer;

	/** The enabled. */
	private boolean enabled;
	
	/** The timer entries. */
	private Hashtable<String, FileSetMonitorTask> timerEntries;

	/**
	 * Constructor.
	 */
	public FileMonitor() {
		// Create timer, run timer thread as daemon.
		timer = new Timer(true);
		timerEntries = new Hashtable<String, FileSetMonitorTask>();
		enabled=true;
	}

	/**
	 * Adds a monitored file with a FileChangeListener.
	 *
	 * @param listener          listener to notify when the file changed.
	 * @param file the file
	 * @param period          polling period in milliseconds.
	 * @return the i modification checker
	 */
	public IModificationChecker addFileChangeListener(IFileChangeListener listener, File file, long period) {
		return addFileChangeListener(listener, Arrays.asList(new File[]{file}), period);
	}

	/**
	 * Adds a monitored file with a FileChangeListener.
	 * List<File> filesToLoad
	 *
	 * @param listener          listener to notify when the file changed.
	 * @param files the files
	 * @param period          polling period in milliseconds.
	 * @return the i modification checker
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
		if(enabled) listener.fileChanged(file);
	}

	/**
	 * Checks if is enabled.
	 *
	 * @return true, if is enabled
	 */
	public boolean isEnabled() {
		return enabled;
	}

	/**
	 * Sets the enabled.
	 *
	 * @param enabled the new enabled
	 */
	public void setEnabled(boolean enabled) {
		this.enabled = enabled;
	}

	/**
	 * File monitoring task.
	 */
	class FileSetMonitorTask extends TimerTask implements IModificationChecker{

		/** The listener. */
		IFileChangeListener listener;

		/** The monitored files. */
		private List<File> monitoredFiles;

		/** The last modified times. */
		private List<Long> lastModifiedTimes;

		/** The period. */
		public final long period;

		/**
		 * Instantiates a new file set monitor task.
		 *
		 * @param listener the listener
		 * @param monitoredFiles the monitored files
		 * @param period the period
		 */
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

		/* (non-Javadoc)
		 * @see java.util.TimerTask#run()
		 */
		public void run() {
			check();
		}

		/* (non-Javadoc)
		 * @see com.minres.scviewer.e4.application.internal.util.IModificationChecker#check()
		 */
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
