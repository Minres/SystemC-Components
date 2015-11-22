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
package com.minres.scviewer.e4.application.internal.status;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import javax.inject.Inject;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.core.runtime.jobs.IJobChangeEvent;
import org.eclipse.core.runtime.jobs.Job;
import org.eclipse.core.runtime.jobs.JobChangeAdapter;
import org.eclipse.core.runtime.jobs.ProgressProvider;
import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.di.UISynchronize;
import org.eclipse.e4.ui.model.application.ui.menu.MToolControl;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.jface.action.StatusLineManager;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.ProgressBar;
import org.osgi.service.prefs.PreferencesService;

/**
 * The Class StatusBarControl.
 */
public class StatusBarControl {

	/** The Constant STATUS_UPDATE. */
	public static final String STATUS_UPDATE="StatusUpdate";

	/** The model service. */
	@Inject	EModelService modelService;

	/** The osgi preverences. */
	@Inject	@Optional PreferencesService osgiPreverences;

	/** The sync. */
	private final UISynchronize sync;

	/** The manager. */
	protected StatusLineManager manager;

	/** The monitor. */
	private SyncedProgressMonitor monitor;
	
	/** The progress bar. */
	private ProgressBar progressBar;

	/**
	 * Instantiates a new status bar control.
	 *
	 * @param sync the sync
	 */
	@Inject
	public StatusBarControl(UISynchronize sync) {
		this.sync=sync;
		manager = new StatusLineManager();
		manager.update(true);
	}

	/**
	 * Creates the widget.
	 *
	 * @param parent the parent
	 * @param toolControl the tool control
	 */
	@PostConstruct
	void createWidget(Composite parent, MToolControl toolControl) {
		if (toolControl.getElementId().equals("org.eclipse.ui.StatusLine")) { //$NON-NLS-1$
			createStatusLine(parent, toolControl);
		} else if (toolControl.getElementId().equals("org.eclipse.ui.HeapStatus")) { //$NON-NLS-1$
			createHeapStatus(parent, toolControl);
		} else if (toolControl.getElementId().equals("org.eclipse.ui.ProgressBar")) { //$NON-NLS-1$
			createProgressBar(parent, toolControl);
		}
	}

	/**
	 * Destroy.
	 */
	@PreDestroy
	void destroy() {
		if (manager != null) {
			manager.dispose();
			manager = null;
		}
	}

	/**
	 * Creates the progress bar.
	 *
	 * @param parent the parent
	 * @param toolControl the tool control
	 */
	private void createProgressBar(Composite parent, MToolControl toolControl) {
		new Label(parent, SWT.NONE);
		progressBar = new ProgressBar(parent, SWT.SMOOTH);
		progressBar.setBounds(100, 10, 200, 20);
		new Label(parent, SWT.NONE);
		monitor=new SyncedProgressMonitor(progressBar);
		Job.getJobManager().setProgressProvider(new ProgressProvider() {
			@Override
			public IProgressMonitor createMonitor(Job job) {
				return monitor.addJob(job);
			}
		});
	}

	/**
	 * Creates the heap status.
	 *
	 * @param parent the parent
	 * @param toolControl the tool control
	 */
	private void createHeapStatus(Composite parent, MToolControl toolControl) {
		new HeapStatus(parent, osgiPreverences.getSystemPreferences());
	}

	/**
	 * Creates the status line.
	 *
	 * @param parent the parent
	 * @param toolControl the tool control
	 */
	private void createStatusLine(Composite parent, MToolControl toolControl) {
		//		IEclipseContext context = modelService.getContainingContext(toolControl);
		manager.createControl(parent);
	}

	/**
	 * Gets the status event.
	 *
	 * @param text the text
	 * @return the status event
	 */
	@Inject @Optional
	public void  getStatusEvent(@UIEventTopic(STATUS_UPDATE) String text) {
		if(manager!=null ){
			manager.setMessage(text);
		}
	} 

	/**
	 * The Class SyncedProgressMonitor.
	 */
	private final class SyncedProgressMonitor extends NullProgressMonitor {

		// thread-Safe via thread confinement of the UI-Thread 
		/** The running tasks. */
		// (means access only via UI-Thread)
		private long runningTasks = 0L;
		
		/** The progress bar. */
		private ProgressBar progressBar;

		/**
		 * Instantiates a new synced progress monitor.
		 *
		 * @param progressBar the progress bar
		 */
		public SyncedProgressMonitor(ProgressBar progressBar) {
			super();
			this.progressBar = progressBar;
			runningTasks=0;
			progressBar.setSelection(0);
			progressBar.setEnabled(false);
		}

		/* (non-Javadoc)
		 * @see org.eclipse.core.runtime.NullProgressMonitor#beginTask(java.lang.String, int)
		 */
		@Override
		public void beginTask(final String name, final int totalWork) {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					if(runningTasks <= 0) {  // --- no task is running at the moment ---
						progressBar.setEnabled(false);
						progressBar.setSelection(0);
						progressBar.setMaximum(totalWork);
					} else { // --- other tasks are running ---
						progressBar.setMaximum(progressBar.getMaximum() + totalWork);
					}
					runningTasks++;
					progressBar.setToolTipText("Currently running: " + runningTasks + "\nLast task: " + name);
				}
			});
		}

		/* (non-Javadoc)
		 * @see org.eclipse.core.runtime.NullProgressMonitor#worked(int)
		 */
		@Override
		public void worked(final int work) {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					progressBar.setSelection(progressBar.getSelection() + work);
				}
			});
		}

		/* (non-Javadoc)
		 * @see org.eclipse.core.runtime.NullProgressMonitor#done()
		 */
		@Override
		public void done() {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					progressBar.setSelection(0);
					progressBar.setMaximum(1);
					progressBar.setEnabled(false);
				}
			});
		}

/**
 * Adds the job.
 *
 * @param job the job
 * @return the i progress monitor
 */
/*
		@Override
		public boolean isCanceled() {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					cancelled=delegate.isCanceled();
				}
			});
			return cancelled;
		}

		@Override
		public void setCanceled(final boolean value) {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					delegate.setCanceled(value);
				}
			});
		}
*/
		public IProgressMonitor addJob(Job job){
			if(job != null){
				job.addJobChangeListener(new JobChangeAdapter() {
					@Override
					public void done(IJobChangeEvent event) {
						sync.syncExec(new Runnable() {

							@Override
							public void run() {
								runningTasks--;
								if (runningTasks > 0){	// --- some tasks are still running ---
									progressBar.setToolTipText("Currently running: " + runningTasks);
								} else { // --- all tasks are done (a reset of selection could also be done) ---
									progressBar.setToolTipText("No background progress running.");
								}
							}
						});
						// clean-up
						event.getJob().removeJobChangeListener(this);
					}
				});
			}
			return this;
		}
	}
}