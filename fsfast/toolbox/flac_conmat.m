function flacnew = flac_conmat(flac,nthcon)
% flacnew = flac_conmat(flac,nthcon)
%
% Computes the contrast matrix for the nth contrast. 
% Adds to flac.con(nthcon).C
%
%


%
% flac_conmat.m
%
% Original Author: Doug Greve
% CVS Revision Info:
%    $Author: greve $
%    $Date: 2009/04/17 20:09:46 $
%    $Revision: 1.3.2.1 $
%
% Copyright (C) 2002-2007,
% The General Hospital Corporation (Boston, MA). 
% All rights reserved.
%
% Distribution, usage and copying of this software is covered under the
% terms found in the License Agreement file named 'COPYING' found in the
% FreeSurfer source code root directory, and duplicated here:
% https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferOpenSourceLicense
%
% General inquiries: freesurfer@nmr.mgh.harvard.edu
% Bug reports: analysis-bugs@nmr.mgh.harvard.edu
%

flacnew = [];

if(nargin ~= 2) 
  fprintf('flacnew = flac_conmat(flac,nthcon)\n');
  return;
end

if(nthcon > length(flac.con))
  fprintf('ERROR: flac_conmat: nthcon = %d\n',nthcon);
  return;
end

con = flac.con(nthcon);
nevs = length(flac.ev);

if(con.rmprestim)
  % Remove the prestimulus baseline, added on 4/8/09
  % Handles both sumdelays and nosumdelays (sumevreg)
  % Use params from first EV to compute C for a single condition
  evind = flac_evindex(flac,con.ev(1).name);
  evA = flac.ev(evind);
  if(~strcmp('fir',evA.model))
    fprintf('ERROR: need FIR EV to remove PreStim\n');
    return;
  end
  psdmin = evA.psdwin(1);
  psdmax = evA.psdwin(2);
  dpsd   = evA.psdwin(3);
  taxis = fast_psdwin([psdmin dpsd psdmax],'irftaxis');
  WDelay = ones(1,length(taxis));
  TW = psdmax-psdmin;
  % R is matrix for a single condition
  R = fast_condctrstmtx(dpsd,TW,-psdmin,con.sumevreg,WDelay,1);
  % Number of rows in final C
  J = size(R,1);
  % Now determine C
  C = [];
  for nthev = 1:nevs
    [evw evrw hit] = flac_evconw(flac,nthev,nthcon);    
    if(hit) 
      if(~strcmp('fir',flac.ev(nthev).model))
	fprintf('ERROR: need FIR EV to remove PreStim\n');
	return;
      end
      if(max(abs(flac.ev(nthev).psdwin-evA.psdwin)) > .001)
	fprintf('ERROR: all FIR EVs must have same PSDWin to remove PreStim\n');
	return;
      end
      Cev = evw*R;
    else    
      Cev = zeros(J,flac.ev(nthev).nreg);
    end
    C = [C Cev];
  end
  flacnew = flac;
  flacnew.con(nthcon).C = C;
  return
end


if(con.sumev == 1 & con.sumevreg == 1)
  % t-test, C has only one row
  C = [];
  for nthev = 1:nevs
    [evw evrw hit] = flac_evconw(flac,nthev,nthcon);    
    if(isempty(evw)) return; end
    C = [C evw*evrw];
  end
end

if(con.sumev == 1 & con.sumevreg == 0)
  % Sum the EVs but not the EV Regs
  % Determine J, the number of rows in C
  J = 0;
  for nthev = 1:nevs
    [evw evrw hit] = flac_evconw(flac,nthev,nthcon);    
    if(isempty(evw)) return; end    
    if(hit)
      if(J==0) J = flac.ev(nthev).nreg;
      else
	if(J ~= flac.ev(nthev).nreg)
	  fprintf('ERROR: flac_conmat: %s: (SumEv=1,SumEVReg=0)\n',con.name);
	  fprintf('Two or more EVs have different numbers of regressors\n');
	  return;
	end
      end
    end % if hit
  end % for nthev

  % Now determine C
  C = [];
  for nthev = 1:nevs
    [evw evrw hit] = flac_evconw(flac,nthev,nthcon);    
    if(hit) Cev = evw*diag(evrw);
    else    Cev = zeros(J,flac.ev(nthev).nreg);
    end
    C = [C Cev];
  end

end

if(con.sumev == 0 & con.sumevreg == 1)
  % Determine J, the number of rows in C
  J = 0;
  for nthev = 1:nevs
    [evw evrw hit] = flac_evconw(flac,nthev,nthcon);    
    if(isempty(evw)) return; end
    if(hit) J = J + 1; end
  end 
  % Now determine C
  C = [];
  nthhit = 0;
  for nthev = 1:nevs
    [evw evrw hit] = flac_evconw(flac,nthev,nthcon);    
    Cev = zeros(J,flac.ev(nthev).nreg);
    if(hit)
      nthhit = nthhit + 1;
      Cev(nthhit,:) = evw*evrw;
    end
    C = [C Cev];
  end
end

if(con.sumev == 0 & con.sumevreg == 0)
  % Just put everything on the diagonal and remove the zeros later
  c = [];
  for nthev = 1:nevs
    [evw evrw hit] = flac_evconw(flac,nthev,nthcon);    
    c = [c evw*evrw];
  end 
  C = diag(c);
end

% Now remove rows of C that are all 0
J = size(C,1);
keeprows = [];
for row = 1:J
  ind = find(C(row,:) ~= 0);
  if(~isempty(ind)) keeprows = [keeprows row]; end
end
C = C(keeprows,:);

if(isempty(C))
  fprintf('ERROR: flac_conmat: %s: all weights are 0\n',con.name);
  return;
end

flacnew = flac;
flacnew.con(nthcon).C = C;
return














