package com.minres.scviewer.e4.application.internal;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import javax.inject.Inject;

import org.eclipse.core.internal.preferences.BundleDefaultPreferences;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.jobs.IJobChangeEvent;
import org.eclipse.core.runtime.jobs.Job;
import org.eclipse.core.runtime.jobs.JobChangeAdapter;
import org.eclipse.core.runtime.jobs.ProgressProvider;
import org.eclipse.core.runtime.preferences.IEclipsePreferences;
import org.eclipse.e4.core.di.annotations.Optional;
import org.eclipse.e4.ui.di.UIEventTopic;
import org.eclipse.e4.ui.di.UISynchronize;
import org.eclipse.e4.ui.model.application.ui.menu.MToolControl;
import org.eclipse.e4.ui.workbench.modeling.EModelService;
import org.eclipse.jface.action.StatusLineManager;
import org.eclipse.swt.widgets.Composite;

public class StatusBarControl {

	public static final String STATUS_UPDATE="StatusUpdate";
	public static final String ZOOM_LEVEL="ZoomLevelUpdate";
	public static final String CURSOR_TIME="CursorPosUpdate";

	@Inject	EModelService modelService;
	@Inject	@Optional IEclipsePreferences preferences;
	private final UISynchronize sync;

	protected StatusLineManager manager;

	private SyncedProgressMonitor monitor;

	@Inject
	public StatusBarControl(UISynchronize sync) {
		this.sync=sync;
		manager = new StatusLineManager();
		manager.update(true);
	}

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

	@PreDestroy
	void destroy() {
		if (manager != null) {
			manager.dispose();
			manager = null;
		}
	}

	/**
	 * @param parent
	 * @param toolControl
	 */
	private void createProgressBar(Composite parent, MToolControl toolControl) {
		manager.createControl(parent);
		monitor=new SyncedProgressMonitor(manager.getProgressMonitor());
		Job.getJobManager().setProgressProvider(new ProgressProvider() {
			@Override
			public IProgressMonitor createMonitor(Job job) {
				return monitor.addJob(job);
			}
		});
	}

	/**
	 * @param parent
	 * @param toolControl
	 */
	private void createHeapStatus(Composite parent, MToolControl toolControl) {
		if(preferences==null){
			preferences=new BundleDefaultPreferences();
			preferences.putInt(IHeapStatusConstants.PREF_UPDATE_INTERVAL, 100);
			preferences.putBoolean(IHeapStatusConstants.PREF_SHOW_MAX, true);
		}
		new HeapStatus(parent, preferences);
	}

	/**
	 * @param parent
	 * @param toolControl
	 */
	private void createStatusLine(Composite parent, MToolControl toolControl) {
		//		IEclipseContext context = modelService.getContainingContext(toolControl);
		manager.createControl(parent);
	}

	@Inject @Optional
	public void  getStatusEvent(@UIEventTopic(STATUS_UPDATE) String text) {
		if(manager!=null ){
			manager.setMessage(text);
		}
	} 

	private final class SyncedProgressMonitor implements IProgressMonitor {

		IProgressMonitor delegate;
		private boolean cancelled;

		SyncedProgressMonitor(IProgressMonitor delegate){
			this.delegate=delegate;
		}

		public IProgressMonitor addJob(Job job){
			if(job != null){
				job.addJobChangeListener(new JobChangeAdapter() {
					@Override
					public void done(IJobChangeEvent event) {	            
						// clean-up
						event.getJob().removeJobChangeListener(this);
					}
				});
			}
			return this;
		}

		@Override
		public void beginTask(final String name, final int totalWork) {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					delegate.beginTask(name, totalWork);
				}
			});
		}

		@Override
		public void worked(final int work) {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					delegate.worked(work);
				}
			});
		}

		@Override
		public void done() {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					delegate.done();
				}
			});
		}

		@Override
		public void internalWorked(final double work) {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					delegate.internalWorked(work);
				}
			});
		}

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

		@Override
		public void setTaskName(final String name) {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					delegate.setTaskName(name);
				}
			});
		}

		@Override
		public void subTask(final String name) {
			sync.syncExec(new Runnable() {
				@Override
				public void run() {
					delegate.subTask(name);
				}
			});
		}
	}
}