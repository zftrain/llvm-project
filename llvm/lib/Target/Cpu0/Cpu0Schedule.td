def ALU : FuncUnit;
def IMULDIV : FuncUnit;

def IIAlu : InstrItinClass;
def II_CLO : InstrItinClass;
def II_CLZ : InstrItinClass;
def IILoad : InstrItinClass;
def IIStore : InstrItinClass;
def IIBranch : InstrItinClass;
def IIPseudo : InstrItinClass;

def Cpu0GenericItineraries : ProcessorItineraries<[ALU, IMULDIV], [], [
    InstrItinData<IIAlu, [InstrStage<1, [ALU]>]>,
    InstrItinData<II_CLO, [InstrStage<1, [ALU]>]>,
    InstrItinData<II_CLZ, [InstrStage<1, [ALU]>]>,
    InstrItinData<IILoad, [InstrStage<3, [ALU]>]>,
    InstrItinData<IIStore, [InstrStage<1, [ALU]>]>,
    InstrItinData<IIBranch, [InstrStage<1, [ALU]>]>
]>;