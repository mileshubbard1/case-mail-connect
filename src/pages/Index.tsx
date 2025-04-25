
import { Header } from "@/components/layout/Header";
import { TopNav } from "@/components/layout/TopNav";
import { StatsCard } from "@/components/dashboard/StatsCard";
import { CaseList } from "@/components/cases/CaseList";

const Index = () => {
  return (
    <div className="flex flex-col h-screen bg-gray-50">
      <Header />
      <TopNav />
      
      <main className="flex-1 overflow-auto p-6">
        <div className="max-w-7xl mx-auto space-y-6">
          <StatsCard />
          <div className="h-[700px]">
            <CaseList />
          </div>
        </div>
      </main>
    </div>
  );
};

export default Index;
