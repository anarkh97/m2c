//Rotation of a slotted disk -- a well-known example for testing interface tracking.
//Parameters based on Hartmann, Meinke, Schroder 2008 (Hardcoded in M2C w/ macro LEVELSET_TEST=1)

under Mesh {
  X0   = 0.0;
  Xmax = 100.0;
  Y0   = 0.0;
  Ymax = 100.0;
  Z0   = -2.0e-1;
  Zmax = 2.0e-1;

  BoundaryConditionX0   = Farfield;
  BoundaryConditionXmax = Farfield;
  BoundaryConditionY0   = Farfield;
  BoundaryConditionYmax = Farfield;
  BoundaryConditionZ0   = Symmetry;
  BoundaryConditionZmax = Symmetry;

  NumberOfCellsX = 256;
  NumberOfCellsY = 256;
  NumberOfCellsZ = 1;
}

//NOT REALLY USED
under Equations {
  under Material[0] { //air
    EquationOfState = StiffenedGas;
    under StiffenedGasModel {
      SpecificHeatRatio = 1.4;
      PressureConstant = 0.0;
    }
  }
  under Material[1] { //air
    EquationOfState = StiffenedGas;
    under StiffenedGasModel {
      SpecificHeatRatio = 1.4;
      PressureConstant = 0.0;
    }
  }
}

under InitialCondition {

}

under BoundaryConditions {
  under Inlet {
    Density = 0.0010; 
    VelocityX = 0.0;
    VelocityY = 0.0;
    VelocityZ = 0.0;
    Pressure = 1.0e5;
  }
}

under Space {
  under NavierStokes {
    Flux = HLLC;
    under Reconstruction {
      Type = Linear;    
      VariableType = ConservativeCharacteristic;
      Limiter = GeneralizedMinMod;
      GeneralizedMinModCoefficient = 1.1;
    }
  }

  under LevelSet[0] {
    MaterialID = 1;
    Solver = FiniteDifference;

    BoundaryConditionX0   = LinearExtrapolation;
    BoundaryConditionXmax = LinearExtrapolation;
    BoundaryConditionY0   = LinearExtrapolation;
    BoundaryConditionYmax = LinearExtrapolation;
    BoundaryConditionZ0   = ZeroNeumann;
    BoundaryConditionZmax = ZeroNeumann;

    under Reinitialization {
      Frequency = 104; //so that we get a clean result at the end
      MaxIts = 1000;
      CFL = 0.8;
      ConvergenceTolerance = 1.0e-5;
      FirstLayerTreatment = Fixed;
    }
  }
}

under MultiPhase {
  Flux = Numerical;
  ReconstructionAtInterface = Constant;
  PhaseChange = RiemannSolution;
}

under Time {
  Type = Explicit;
  MaxTime = 628.0;
  TimeStep = 0.3;
  under Explicit {
    Type = RungeKutta2;
  }
}

under Output {
  Prefix = "results/";
  Solution = "solution";
  TimeInterval = 30.0;
  Velocity = On;
  LevelSet0 = On;
  MaterialID = On;

  //VerboseScreenOutput = On;
}
